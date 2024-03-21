#include "general.h"
#include "target.h"
#include "target_internal.h"
#include "cortexm.h"
#include "adiv5.h"

static bool ch579_flash_erase(target_flash_s *f, target_addr_t addr, size_t len)
{
    DEBUG_INFO("ch579 flash erase\n");
    return false;
}
static bool ch579_flash_write(target_flash_s *f, target_addr_t dest, const void *src, size_t len)
{
    DEBUG_INFO("ch579 flash write\n");
    return false;
}
static bool ch579_flash_prepare(target_flash_s *f)
{
    DEBUG_INFO("ch579 flash prepare\n");
    return false;
}
static bool ch579_flash_done(target_flash_s *f)
{
    DEBUG_INFO("ch579 flash done\n");
    return false;
}

bool ch579_probe(target_s *target)
{
    DEBUG_INFO("ch579 probe\n");
    uint8_t chip_id = target_mem_read8(target, 0x40001041);
    if (chip_id != 0x79) {
        DEBUG_ERROR("Not CH579! 0x%02x\n", chip_id);
        return false;
    }

    target->driver = "CH579";

	target_flash_s *f = calloc(1, sizeof(*f));
	if (!f) { /* calloc failed: heap exhaustion */
		DEBUG_ERROR("calloc: failed in %s\n", __func__);
		return false;
	}
	f->start = 0;
	f->length = 0x3f000;
	f->blocksize = 512;
	f->writesize = 512;
	f->erase = ch579_flash_erase;
	f->write = ch579_flash_write;
	f->prepare = ch579_flash_prepare;
	f->done = ch579_flash_done;
	f->erased = 0xff;
	target_add_flash(target, f);

    target_add_ram(target, 0x20000000, 0x8000);
    return true;
}
