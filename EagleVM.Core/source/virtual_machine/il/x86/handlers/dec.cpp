#include "eaglevm-core/virtual_machine/il/x86/handlers/dec.h"

namespace eagle::il::handler
{
    dec::dec()
    {
        entries = {
            {codec::gpr_64, 1},
            {codec::gpr_32, 1},
            {codec::gpr_16, 1},
        };
    }
}
