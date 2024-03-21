#include "general.h"
#include "target.h"
#include "target_internal.h"
#include "cortexm.h"
#include "adiv5.h"
#include "buffer_utils.h"

static bool ch579_wait_flash(target_s *const t, platform_timeout_s *const timeout)
{
    uint16_t x;
	while (((x = target_mem_read16(t, 0x4000180a)) & 0xFF) != 0x40) {
        DEBUG_INFO("ch579 wait %04x\n", x);
		if (target_check_error(t))
			return false;
		if (timeout)
			target_print_progress(timeout);
	}
	return true;
}

static bool ch579_flash_erase(target_flash_s *f, target_addr_t addr, size_t len)
{
    (void)len;
    DEBUG_INFO("ch579 flash erase %08x\n", (int)addr);
    target_mem_write32(f->t, 0x40001804, addr);
    if (addr < 0x40000) {
        target_mem_write8(f->t, 0x40001808, 0xa6);
    } else {
        // DANGER: this erases the options/info page, which can brick the chip
        DEBUG_INFO("ch579: warranty voided!\n");
        target_mem_write8(f->t, 0x40001808, 0xa5);
    }

    if (!ch579_wait_flash(f->t, NULL))
        return false;
    return true;
}
static bool ch579_flash_write(target_flash_s *f, target_addr_t dest, const void *src, size_t len)
{
    (void)len;
    DEBUG_INFO("ch579 flash write %08x\n", (int)dest);
    target_mem_write32(f->t, 0x40001804, dest);
    target_mem_write32(f->t, 0x40001800, read_le4((const uint8_t *)src, 0));
    if (dest < 0x40000) {
        target_mem_write8(f->t, 0x40001808, 0x9a);
    } else {
        // DANGER: this erases the options/info page, which can brick the chip
        DEBUG_INFO("ch579: warranty voided!\n");
        target_mem_write8(f->t, 0x40001808, 0x99);
    }

    if (!ch579_wait_flash(f->t, NULL))
        return false;
    return true;
}
static bool ch579_flash_prepare(target_flash_s *f)
{
    DEBUG_INFO("ch579 flash prepare\n");
    uint8_t glob_cfg_info = target_mem_read8(f->t, 0x40001045);
    if (glob_cfg_info & (1 << 5))
    {
	    tc_printf(f->t, "Flash operations not permitted if bootloader is enabled!\n");
        return false;
    }
    // just enable both write flags now, so that code/data flash can be treated as contiguous
    target_mem_write8(f->t, 0x40001809, 0b10001100);
    return true;
}
static bool ch579_flash_done(target_flash_s *f)
{
    DEBUG_INFO("ch579 flash done\n");
    target_mem_write8(f->t, 0x40001809, 0b10000000);
    return true;
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
	f->writesize = 4;
	f->erase = ch579_flash_erase;
	f->write = ch579_flash_write;
	f->prepare = ch579_flash_prepare;
	f->done = ch579_flash_done;
	f->erased = 0xff;
	target_add_flash(target, f);

    target_add_ram(target, 0x20000000, 0x8000);
    return true;
}
