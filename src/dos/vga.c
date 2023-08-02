#include <string.h>
#include "vga.h"
#include "vgaregs.h"
#include "cdpmi.h"
#include "dosutil.h"

static void crtc_write(int reg, unsigned char val);
static unsigned char crtc_read(int reg);

int vga_setmode(int mode)
{
	struct dpmi_regs regs = {0};

	regs.eax = mode;	/* func 00 | mode */
	dpmi_int(0x10, &regs);
	return 0;
}

static unsigned short crtc_modex_regs[] = {
	0x0d06,	/* vertical total */
	0x3e07,	/* vcount overflow bit */
	0x4109,	/* double-scan */
	0xea10,	/* vsync start */
	0xac11,	/* vsync end & protect */
	0xdf12,	/* vertical visible */
	0x0014,	/* no dword mode */
	0xe715,	/* vblank start */
	0x0616,	/* vblank end */
	0xe317,	/* byte mode */
	0
};

int vga_setmodex(void)
{
	int i;
	unsigned char val;

	vga_setmode(0x13);

	/* disable chain-4 (C4=0, O/E=1 (sequential), EM=1 (extmem), A/G=0 (gfx) */
	outpw(VGA_SC_ADDR_PORT, VGA_SC_MEMMODE_REG | 0x0600);
	/* pull reset low */
	outpw(VGA_SC_ADDR_PORT, VGA_SC_RESET_REG | 0x0100);
	/* 25mhz dot clock, 60hz scan */
	outp(VGA_MISC_PORT, VGA_MISC_480 | VGA_MISC_PG1 | VGA_MISC_CLK25 |
			VGA_MISC_CPUEN | VGA_MISC_COLOR);
	/* return reset high */
	outpw(VGA_SC_ADDR_PORT, VGA_SC_RESET_REG | 0x0300);

	/* disable CRTC write-protect */
	crtc_write(CRTC_VRETEND_REG, crtc_read(CRTC_VRETEND_REG) & ~CRTC_VRETEND_PR);
	/* change CRTC registers */
	for(i=0; crtc_modex_regs[i]; i++) {
		outpw(VGA_CRTC_PORT, crtc_modex_regs[i]);
	}

	vga_planemask(0xf);
	memset(VGA_FBADDR, 3, 320 * 240 / 4);
	return 0;
}

static void crtc_write(int reg, unsigned char val)
{
	outpw(VGA_CRTC_ADDR_PORT, reg | ((unsigned int)val << 8));
}

static unsigned char crtc_read(int reg)
{
	outp(VGA_CRTC_ADDR_PORT, reg);
	return inp(VGA_CRTC_DATA_PORT);
}
