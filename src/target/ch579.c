#include "general.h"
#include "target.h"
#include "target_internal.h"
#include "cortexm.h"
#include "adiv5.h"

bool ch579_probe(target_s *target)
{
    DEBUG_INFO("ch579 probe\n");
    uint8_t chip_id = target_mem_read8(target, 0x40001041);
    if (chip_id != 0x79) {
        DEBUG_ERROR("Not CH579! 0x%02x\n", chip_id);
        return false;
    }

    target->driver = "CH579";
    return true;
}
