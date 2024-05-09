#pragma once
#include <set>
#include <unordered_map>
#include "commands/cmd_exit.h"

#include "eaglevm-core/disassembler/basic_block.h"
#include "eaglevm-core/virtual_machine/ir/block.h"
#include "eaglevm-core/virtual_machine/ir/x86/base_handler_gen.h"

namespace eagle::dasm
{
    class segment_dasm;
}

namespace eagle::il
{
    class ir_preopt_block;
    using ir_preopt_block_ptr = std::shared_ptr<ir_preopt_block>;

    class ir_translator
    {
    public:
        ir_translator(dasm::segment_dasm* seg_dasm);

        std::vector<ir_preopt_block_ptr> translate();

        std::vector<block_il_ptr> get_optimized();
        std::vector<block_il_ptr> get_unoptimized();

    private:
        dasm::segment_dasm* dasm;
        std::unordered_map<dasm::basic_block*, ir_preopt_block_ptr> bb_map;

        ir_preopt_block_ptr translate_block(dasm::basic_block* bb);
    };

    class ir_preopt_block
    {
    public:
        void init();

        block_il_ptr get_entry();
        std::vector<block_il_ptr>& get_body();

        void add_body(const block_il_ptr& block);

    private:
        block_il_ptr entry;
        std::vector<block_il_ptr> body;
    };
}