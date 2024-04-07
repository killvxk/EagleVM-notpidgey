#include "eaglevm-core/virtual_machine/il/x86/handlers/inc.h"

namespace eagle::il::handler
{
    inc::inc()
    {
        handlers = {
            { codec::gpr_64, 1 },
            { codec::gpr_32, 1 },
            { codec::gpr_16, 1 },
        };
    }
}
