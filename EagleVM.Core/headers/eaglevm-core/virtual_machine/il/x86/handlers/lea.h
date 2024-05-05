#pragma once
#include "eaglevm-core/virtual_machine/il/x86/base_handler_gen.h"
#include "eaglevm-core/virtual_machine/il/x86/base_x86_lifter.h"

namespace eagle::il::handler
{
    class lea : public base_handler_gen
    {
    public:
        lea();
        il_insts gen_handler(codec::reg_class size, uint8_t operands) override;
    };
}

namespace eagle::il::lifter
{
    class lea : public base_x86_lifter
    {
        using base_x86_lifter::base_x86_lifter;

        bool virtualize_as_address(codec::dec::operand operand, uint8_t idx) override;
        void finalize_translate_to_virtual() override;
        bool skip(uint8_t idx) override;
    };
}