#include <string.h>
#include <conio.h>
#include <i86.h>
#include "vidsys.h"
#include "drv.h"
#include "vga.h"
#include "logger.h"

static int init(void);
static void cleanup(void);
static int setmode(int mode);
static int curmode(void);

static void setpal4(int idx, int count, const struct vid_color *col);
static void getpal4(int idx, int count, struct vid_color *col);
static void clear4(uint32_t color);
static void blitfb4(void *fb, int pitch);
static void fill4(int x, int y, int w, int h, uint32_t color);

static void clear8(uint32_t color);
static void blitfb8(void *fb, int pitch);
static void fill8(int x, int y, int w, int h, uint32_t color);


static struct vid_driver drv;
static struct vid_drvops drvops = {init, cleanup, setmode, curmode};
static struct vid_modeinfo modes[] = {
	{3, 80, 25, 4},
	{0x12, 640, 480, 4},
	{0x13, 320, 200, 8}
};

static struct vid_gfxops gfxops_mode12h = {
	0, 0, setpal4, getpal4, vid_vsync, clear4, blitfb4, 0, fill4 };
static struct vid_gfxops gfxops_mode13h = {
	0, 0, vga_setpal, vga_getpal, vid_vsync, clear8, blitfb8, 0, fill8 };

void vid_register_vga(void)
{
	int i;

	drv.name = "vga";
	drv.prio = 1;
	drv.ops = &drvops;
	drv.modes = modes;
	drv.num_modes = sizeof modes / sizeof *modes;

	for(i=0; i<drv.num_modes; i++) {
		modes[i].drv = &drv;
		modes[i].vmem_addr = 0xa0000;

		switch(modes[i].modeno) {
		case 0x3:
			modes[i].vmem_addr = 0xb8000;
			break;

		case 0x13:
			modes[i].ops = gfxops_mode13h;
			break;

		case 0x12:
			modes[i].ops = gfxops_mode12h;
			break;
		}
	}

	vid_drvlist[vid_numdrv++] = &drv;
}

void vid_vsync(void)
{
	while(inp(VGA_STAT1_PORT) & 8);
	while((inp(VGA_STAT1_PORT) & 8) == 0);
}

void vga_setpal(int idx, int count, const struct vid_color *col)
{
	int i;
	outp(VGA_DAC_WADDR_PORT, idx);
	for(i=0; i<count; i++) {
		outp(VGA_DAC_DATA_PORT, col->r >> 2);
		outp(VGA_DAC_DATA_PORT, col->g >> 2);
		outp(VGA_DAC_DATA_PORT, col->b >> 2);
		col++;
	}
}

void vga_getpal(int idx, int count, struct vid_color *col)
{
	int i;
	outp(VGA_DAC_RADDR_PORT, idx);
	for(i=0; i<count; i++) {
		col->r = inp(VGA_DAC_DATA_PORT) << 2;
		col->g = inp(VGA_DAC_DATA_PORT) << 2;
		col->b = inp(VGA_DAC_DATA_PORT) << 2;
		col++;
	}
}


static int init(void)
{
	return 0;
}

static void cleanup(void)
{
}

static int setmode(int mode)
{
	union REGS regs = {0};
	regs.w.ax = mode;
	int386(0x10, &regs, &regs);
	return 0;
}

static int curmode(void)
{
	union REGS regs = {0};
	regs.w.ax = 0xf00;
	int386(0x10, &regs, &regs);
	return regs.x.eax & 0xff;
}

static void setpal4(int idx, int count, const struct vid_color *col)
{
}

static void getpal4(int idx, int count, struct vid_color *col)
{
}

static void clear4(uint32_t color)
{
}

static void blitfb4(void *fb, int pitch)
{
}

static void fill4(int x, int y, int w, int h, uint32_t color)
{
}

static void clear8(uint32_t color)
{
	memset((void*)0xa0000, color, 64000);
}

static void blitfb8(void *fb, int pitch)
{
	int i;
	unsigned char *src = fb;
	unsigned char *dest = (unsigned char*)0xa0000;
	for(i=0; i<200; i++) {
		memcpy(dest, src, 320);
		dest += 320;
		src += pitch;
	}
}

static void fill8(int x, int y, int w, int h, uint32_t color)
{
	int i;
	unsigned char *fbptr = (unsigned char*)0xa0000;

	fbptr += y * 320 + x;
	for(i=0; i<h; i++) {
		memset(fbptr, color, w);
		fbptr += 320;
	}
}
