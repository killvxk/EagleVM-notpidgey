#include "eaglevm-core/virtual_machine/ir/x86/handlers/jcc.h"

#include "eaglevm-core/virtual_machine/ir/commands/cmd_rflags_load.h"
#include "eaglevm-core/virtual_machine/ir/dynamic_encoder/encoder.h"
#define HS(x) hash_string::hash(x)

namespace eagle::ir
{
    namespace handler
    {
        jcc::jcc()
        {
            build_options = {
                { { ir_size::bit_32 }, "jo rel32" },
                { { ir_size::bit_32 }, "jno rel32" },

                { { ir_size::bit_32 }, "js rel32" },
                { { ir_size::bit_32 }, "jns rel32" },

                { { ir_size::bit_32 }, "je rel32" },
                { { ir_size::bit_32 }, "jne rel32" },

                { { ir_size::bit_32 }, "jc rel32" },
                { { ir_size::bit_32 }, "jnc rel32" },

                { { ir_size::bit_32 }, "jl rel32" },
                { { ir_size::bit_32 }, "jnl rel32" },

                { { ir_size::bit_32 }, "jle rel32" },
                { { ir_size::bit_32 }, "jnle rel32" },

                { { ir_size::bit_32 }, "jp rel32" },
                { { ir_size::bit_32 }, "jpo rel32" },

                { { ir_size::bit_32 }, "jcxz rel32" },
                { { ir_size::bit_32 }, "jecxz rel32" },
            };
        }

        ir_insts jcc::gen_handler(const uint64_t target_handler_id)
        {
            // method inspired by vmprotect 2
            // https://blog.back.engineering/21/06/2021/#vmemu-virtual-branching

            VM_ASSERT("must by generated by vm implementer");
        }
    }

    namespace lifter
    {
        bool jcc::translate_to_il(const uint64_t original_rva, const x86_cpu_flag flags)
        {
            const auto [condition, inverted] = get_exit_condition(static_cast<codec::mnemonic>(inst.mnemonic));
            switch(condition)
            {
                case exit_condition::none:
                    break;
                case exit_condition::jo:
                    break;
                case exit_condition::js:
                    break;
                case exit_condition::je:
                    break;
                case exit_condition::jb:
                    break;
                case exit_condition::jbe:
                    break;
                case exit_condition::jl:
                    break;
                case exit_condition::jle:
                    break;
                case exit_condition::jp:
                    break;
                case exit_condition::jcxz:
                    break;
                case exit_condition::jecxz:
                    break;
                case exit_condition::jrcxz:
                    break;
                case exit_condition::jmp:
                    break;
            }

            block->push_back(std::make_shared<cmd_branch>({ }, condition, inverted));

            return true;
        }

        std::pair<exit_condition, bool> jcc::get_exit_condition(const codec::mnemonic mnemonic)
        {

        }

        void jcc::write_basic_jump(uint64_t jcc_mask, bool pop_targets)
        {
            std::array<il_exit_result, 2> exits;

            long index;
            _BitScanForward(&index, jcc_mask);

            codec::mnemonic mnemonic;
            if (index > 3)
                mnemonic = codec::m_shl;
            else
                mnemonic = codec::m_shr;

            block->push_back({
                // push the two branches
                std::shared_ptr<cmd_push>(exits[0]),
                std::shared_ptr<cmd_push>(exits[1]),

                // load current rsp
                std::make_shared<cmd_push>(reg_vm::vsp, ir_size::bit_64),

                // load flags onto the stack
                std::shared_ptr<cmd_rflags_load>(),

                // mask the single flag
                std::make_shared<cmd_push>(jcc_mask),
                // TODO: std::make_shared<cmd_handler_call>( cmd_and )

                // shift right/left to bit 3
                std::make_shared<cmd_push>(std::abs(8 - index)),
                // TODO: std::make_shared<cmd_handler_call>( cmd_sh? )

                // add: rsp + disp
                std::make_shared<cmd_handler_call>(codec::m_add, handler_sig{ ir_size::bit_64, ir_size::bit_64 }),

                // read [rsp + disp]
                std::make_shared<cmd_mem_read>(ir_size::bit_64),

                // jmp to read
                std::make_shared<cmd_branch>()
            });
        }

        void jcc::write_compare_jump(uint64_t flag_mask_one, uint64_t flag_mask_two, bool pop_targets)
        {
        }
    }
}
