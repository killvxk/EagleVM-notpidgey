#include "eaglevm-core/virtual_machine/ir/x86/handlers/dec.h"

#include "eaglevm-core/virtual_machine/ir/commands/cmd_rflags_load.h"
#include "eaglevm-core/virtual_machine/ir/commands/cmd_rflags_store.h"

namespace eagle::il::handler
{
    dec::dec()
    {
        entries = {
            { { codec::op_none, codec::bit_8 } },
            { { codec::op_none, codec::bit_16 } },
            { { codec::op_none, codec::bit_32 } },
            { { codec::op_none, codec::bit_64 } },
        };
    }

    il_insts dec::gen_handler(codec::reg_class size, uint8_t operands)
    {
        const il_size target_size = static_cast<il_size>(get_reg_size(size));
        const reg_vm vtemp = get_bit_version(reg_vm::vtemp, target_size);

        return {
            std::make_shared<cmd_vm_pop>(vtemp, target_size),
            std::make_shared<cmd_x86_dynamic>(codec::m_dec, vtemp),
            std::make_shared<cmd_vm_push>(vtemp, target_size)
        };
    }
}

namespace eagle::il::lifter
{
    void dec::finalize_translate_to_virtual()
    {
        block->add_command(std::make_shared<cmd_rflags_load>());
        base_x86_translator::finalize_translate_to_virtual();
        block->add_command(std::make_shared<cmd_rflags_store>());
    }
}
