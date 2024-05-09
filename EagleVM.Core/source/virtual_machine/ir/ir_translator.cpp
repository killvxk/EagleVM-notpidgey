#include "eaglevm-core/virtual_machine/ir/ir_translator.h"

#include "eaglevm-core/virtual_machine/ir/commands/include.h"
#include "eaglevm-core/virtual_machine/ir/block.h"

#include "eaglevm-core/disassembler/disassembler.h"
#include "eaglevm-core/codec/zydis_helper.h"
#include "eaglevm-core/virtual_machine/ir/x86/handler_data.h"

namespace eagle::il
{
    ir_translator::ir_translator(dasm::segment_dasm* seg_dasm)
    {
        dasm = seg_dasm;
    }

    std::vector<block_vm_il_ptr> ir_translator::translate()
    {
        // we want to initialzie the entire map with bb translates
        for (dasm::basic_block* block : dasm->blocks)
        {
            const block_vm_il_ptr vm_il = std::make_shared<ir_preopt_block>();
            vm_il->init();

            bb_map[block] = vm_il;
        }

        std::vector<block_vm_il_ptr> result;
        for (dasm::basic_block* block : dasm->blocks)
            result.push_back(translate_block(block));

        return result;
    }

    block_vm_il_ptr ir_translator::translate_block(dasm::basic_block* bb)
    {
        block_vm_il_ptr block_info = bb_map[bb];
        if (bb->decoded_insts.empty())
            return block_info;

        //
        // entry
        //
        const block_il_ptr entry = block_info->get_entry();
        entry->add_command(std::make_shared<cmd_enter>());

        //
        // body
        //
        bool current_vm_block = true;
        block_il_ptr current_block = std::make_shared<block_il>(false);

        // we calculate skips here because a basic block might end with a jump
        // we will handle that manually instead of letting the il translator handle this
        const dasm::block_end_reason end_reason = bb->get_end_reason();
        const uint8_t skips = end_reason == dasm::block_end ? 0 : 1;

        for (uint32_t i = 0; i < bb->decoded_insts.size() - skips; i++)
        {
            // use il x86 translator to translate the instruction to il
            auto decoded_inst = bb->decoded_insts[i];
            auto& [inst, ops] = decoded_inst;

            std::vector<handler_op> il_operands;
            handler::base_handler_gen_ptr handler_gen = nullptr;
            handler::handler_info_ptr target_handler = nullptr;

            codec::mnemonic mnemonic = static_cast<codec::mnemonic>(inst.mnemonic);
            if (instruction_handlers.contains(mnemonic))
            {
                // first we verify if there is even a valid handler for this inustruction
                // we do this by checking the handler generator for this specific handler
                handler_gen = instruction_handlers[mnemonic];

                for (int j = 0; j < inst.operand_count_visible; j++)
                {
                    il_operands.push_back({
                        static_cast<codec::op_type>(ops[i].type),
                        static_cast<codec::reg_size>(ops[i].size)
                    });
                }

                target_handler = handler_gen->get_operand_handler(il_operands);
            }

            bool translate_sucess = target_handler != nullptr;
            if (target_handler)
            {
                // we know that a valid handler exists for this instruction
                // this means that we can bring it into the base x86 lifter

                // now we need to find a lifter
                const uint64_t current_rva = bb->get_index_rva(i);

                auto create_lifter = instruction_lifters[mnemonic];
                const std::shared_ptr<lifter::base_x86_translator> lifter = create_lifter(decoded_inst, current_rva);

                translate_sucess = lifter->translate_to_il(current_rva);
                if (translate_sucess)
                {
                    // append basic block
                    block_il_ptr result_block = lifter->get_block();

                    // TODO: add way to scatter blocks instead of appending them to a single block
                    // this should probably be done post gen though
                    if (!current_vm_block)
                    {
                        // the current block is a x86 block
                        const block_il_ptr previous = current_block;
                        block_info->add_body(current_block);

                        current_block = std::make_shared<block_il>(false);
                        previous->set_exit(std::make_shared<cmd_exit>(current_block, exit_condition::jump));
                        current_vm_block = true;
                    }

                    current_block->copy_from(result_block);
                }
            }

            if (!translate_sucess)
            {
                if (current_vm_block)
                {
                    if (current_block->get_command_count() == 0)
                    {
                        current_vm_block = false;
                    }
                    else
                    {
                        // the current block is a vm block
                        const block_il_ptr previous = current_block;
                        block_info->add_body(current_block);

                        current_block = std::make_shared<block_il>(true);
                        previous->set_exit(std::make_shared<cmd_exit>(current_block, exit_condition::jump));
                        current_vm_block = false;
                    }
                }

                // handler does not exist
                // we need to execute the original instruction

                // todo: if rip relative, translate
                current_block->add_command(std::make_shared<cmd_x86_exec>(decoded_inst));
            }
        }

        //
        // exit
        //
        std::vector<il_exit_result> exits;
        exit_condition condition = exit_condition::none;

        switch (end_reason)
        {
            case dasm::block_jump:
            case dasm::block_end:
            {
                auto [target, type] = dasm->get_jump(bb);
                if (type == dasm::jump_outside_segment)
                    exits.push_back(target);
                else
                    exits.push_back(bb_map[dasm->get_block(target)]->get_entry());

                condition = exit_condition::jump;
                break;
            }
            case dasm::block_conditional_jump:
            {
                // case 1 - condition succeed
                {
                    auto [target, type] = dasm->get_jump(bb);
                    if (type == dasm::jump_outside_segment)
                        exits.push_back(target);
                    else
                        exits.push_back(bb_map[dasm->get_block(target)]->get_entry());
                }

                // case 2 - fall through
                {
                    auto [target, type] = dasm->get_jump(bb, true);
                    if (type == dasm::jump_outside_segment)
                        exits.push_back(target);
                    else
                        exits.push_back(bb_map[dasm->get_block(target)]->get_entry());
                }

                condition = exit_condition::conditional;
                break;
            }
        }

        current_block->set_exit(std::make_shared<cmd_exit>(exits, condition));
        block_info->add_body(current_block);

        return block_info;
    }

    std::vector<block_il_ptr> ir_translator::get_optimized()
    {
        // the idea of this function is that we want to walk every block
        // for every block, we want to check the block that is being jumped to
        // we want to see if vmenter is ever being used for that block
        // if its not, it can be optimized out and vmexits for blocks that jump to it can be removed as well
    }

    std::vector<block_il_ptr> ir_translator::get_unoptimized()
    {
        std::vector<block_il_ptr> final_blocks;
        for (auto& [_, vm_block] : bb_map)
        {
            std::vector<block_il_ptr> body = vm_block->get_body();
            if(body.empty())
                continue;

            if(block_il_ptr entry = vm_block->get_entry())
                final_blocks.push_back(entry);

            final_blocks.append_range(body);
        }

        return final_blocks;
    }

    void ir_preopt_block::init()
    {
        entry = std::make_shared<block_il>();
    }

    block_il_ptr ir_preopt_block::get_entry()
    {
        return entry;
    }

    std::vector<block_il_ptr>& ir_preopt_block::get_body()
    {
        return body;
    }

    void ir_preopt_block::add_body(const block_il_ptr& block)
    {
        body.push_back(block);
    }
}
