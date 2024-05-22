#pragma once
#include <set>
#include <unordered_map>
#include "commands/cmd_branch.h"

#include "eaglevm-core/disassembler/basic_block.h"
#include "eaglevm-core/virtual_machine/ir/block.h"
#include "eaglevm-core/virtual_machine/ir/x86/base_handler_gen.h"

namespace eagle::dasm
{
    class segment_dasm;
}

namespace eagle::ir
{
    class ir_preopt_block;
    using ir_preopt_block_ptr = std::shared_ptr<ir_preopt_block>;
    using ir_preopt_block_vec = std::vector<ir_preopt_block_ptr>;

    class ir_translator
    {
    public:
        ir_translator(dasm::segment_dasm* seg_dasm);

        std::vector<ir_preopt_block_ptr> translate();
        std::vector<block_il_ptr> optimize();

        dasm::basic_block* map_basic_block(const ir_preopt_block_ptr& preopt_target);
        ir_preopt_block_ptr map_preopt_block(dasm::basic_block* basic_block);

    private:
        dasm::segment_dasm* dasm;

        std::unordered_map<dasm::basic_block*, ir_preopt_block_ptr> bb_map;

        ir_preopt_block_ptr translate_block(dasm::basic_block* bb);
        exit_condition get_exit_condition(codec::mnemonic mnemonic);
    };

    using ir_vm_x86_block = std::pair<block_il_ptr, bool>;

    class ir_preopt_block
    {
    public:
        void init();

        block_il_ptr get_entry();
        std::vector<ir_vm_x86_block> get_body();
        block_il_ptr get_exit();

        void add_body(const block_il_ptr& block, bool is_vm);

    private:
        block_il_ptr entry;
        std::vector<ir_vm_x86_block> body;
        block_il_ptr exit;
    };
}