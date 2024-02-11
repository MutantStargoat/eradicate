#include <string.h>
#include <conio.h>
#include <i86.h>
#include "vidsys.h"
#include "drv.h"
#include "vbe.h"
#include "vga.h"
#include "cdpmi.h"

#define farptr_to_linear(rmaddr) \
	((((intptr_t)(rmaddr) >> 12) & 0xffff0) + ((intptr_t)(rmaddr) & 0xffff))

static int init(void);
static void cleanup(void);
static struct vid_modeinfo *find_mode(int mode);
static int setmode(int mode);
static int getmode(void);
static const char *memsize_str(long sz);
static int get_mode_info(int mode, struct vbe_mode_info *mi);
static int conv_vbeinfo(int mode, struct vid_modeinfo *mi, struct vbe_mode_info *vbemi);
static unsigned int calc_mask(int nbits, int pos);
static void print_mode_info(int mode, struct vid_modeinfo *mi);

static void pack(uint32_t *pix, int r, int g, int b);
static void unpack(uint32_t pix, int *r, int *g, int *b);
static void clear(uint32_t color);
static void blit_lfb(int x, int y, int w, int h, void *fb, int pitch);
static void blit_banked(int x, int y, int w, int h, void *fb, int pitch);
static void blitfb_lfb(void *fb, int pitch);
static void blitfb_banked(void *fb, int pitch);
static void flip(int vsync);

static struct vid_driver drv;
static struct vid_drvops drvops = {init, cleanup, setmode, getmode};
static unsigned int vbe_ver;

static int cur_mode;
static struct vid_modeinfo *cur_mi;
static int cur_pgsize;


void vid_register_vbe(void)
{
	drv.name = "vbe";
	drv.prio = 2;
	drv.ops = &drvops;

	vid_drvlist[vid_numdrv++] = &drv;
}


static int init(void)
{
	struct dpmi_regs regs = {0};
	struct vbe_info *vbe;
	struct vbe_mode_info vbemi;
	unsigned short bufseg;
	uint16_t *vbe_modelist, *modelist;
	int i, count;
	struct vid_modeinfo modeinf;

	cur_mode = -1;
	cur_mi = 0;

	vbe = dpmi_lowbuf();
	bufseg = (intptr_t)vbe >> 4;

	/* call VBE function 00 (get controller information) */
	memcpy(vbe->sig, "VBE2", 4);	/* denote we want VBE 2.0 info */
	regs.eax = 0x4f00;
	regs.es = bufseg;
	dpmi_rmint(0x10, &regs);
	if((regs.eax & 0xffff) != 0x4f || memcmp(vbe->sig, "VESA", 4) != 0) {
		fprintf(stderr, "failed to get VBE controller information\n");
		return -1;
	}

	vbe_ver = vbe->ver;

	printf("Found VBE %d.%d\n", VBE_VER_MAJOR(vbe_ver), VBE_VER_MINOR(vbe_ver));
	printf("OEM: %s\n", (char*)farptr_to_linear(vbe->oem_name));
	if(vbe_ver >= 0x0200) {
		printf("%s - %s (%s)\n", (char*)farptr_to_linear(vbe->vendor),
				(char*)farptr_to_linear(vbe->product),
				(char*)farptr_to_linear(vbe->revstr));
	}
	printf("Video RAM: %s\n", memsize_str((long)vbe->vmem_blk * 65536));

	vbe_modelist = (uint16_t*)farptr_to_linear(vbe->modelist_addr);
	count = 0;
	for(i=0; i<1024; i++) {
		if(vbe_modelist[i] == 0xffff) break;
		count++;
	}

	if(!(modelist = malloc(count * sizeof *modelist))) {
		fprintf(stderr, "failed to allocate mode list\n");
		return -1;
	}
	for(i=0; i<count; i++) {
		modelist[i] = vbe_modelist[i];
	}

	if(!(drv.modes = malloc(count * sizeof *drv.modes))) {
		fprintf(stderr, "failed to allocate mode list\n");
		free(modelist);
		return -1;
	}

	drv.num_modes = 0;
	for(i=0; i<count; i++) {
		if(get_mode_info(modelist[i], &vbemi) == -1) {
			continue;
		}
		if(conv_vbeinfo(modelist[i], drv.modes + drv.num_modes, &vbemi) == -1) {
			continue;
		}
		drv.num_modes++;
	}

	free(modelist);
	return 0;
}

static void cleanup(void)
{
	free(drv.modes);
	drv.modes = 0;
	drv.num_modes = 0;
}

static struct vid_modeinfo *find_mode(int mode)
{
	int i;
	for(i=0; i<drv.num_modes; i++) {
		if(drv.modes[i].modeno == mode) {
			return drv.modes + i;
		}
	}
	return 0;
}

static int setmode(int mode)
{
	struct vid_modeinfo *minf;
	struct dpmi_regs regs = {0};

	if((minf = find_mode(mode)) && minf->lfb) {
		mode |= VBE_MODE_LFB;
	}

retry:
	regs.eax = 0x4f02;
	regs.ebx = mode;
	dpmi_rmint(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		if(mode & VBE_MODE_LFB) {
			mode &= ~VBE_MODE_LFB;
			goto retry;
		}
		return -1;
	}
	cur_mode = mode;

	if(!(cur_mi = minf)) return 0;

	cur_pgsize = minf->height * minf->pitch;

	if(mode & VBE_MODE_LFB) {
		minf->ops.blit = blit_lfb;
		minf->ops.blitfb = blitfb_lfb;
	} else {
		minf->ops.blit = blit_banked;
		minf->ops.blitfb = blitfb_banked;
	}

	print_mode_info(mode, minf);
	return 0;
}

static int getmode(void)
{
	return cur_mode;
}

static const char *memsize_str(long sz)
{
	static const char *suffix[] = {"bytes", "kb", "mb", "gb", 0};
	static int cnt = 0;
	static char buf[64];

	while(sz > 1024 && suffix[cnt + 1]) {
		sz >>= 10;
		cnt++;
	}

	sprintf(buf, "%ld %s", sz, suffix[cnt]);
	return buf;
}

static int get_mode_info(int mode, struct vbe_mode_info *mi)
{
	struct dpmi_regs regs = {0};
	struct vbe_mode_info *miptr;
	uint16_t bufseg;

	miptr = dpmi_lowbuf();
	bufseg = (intptr_t)miptr >> 4;

	regs.eax = 0x4f01;
	regs.ecx = mode;
	regs.es = bufseg;
	dpmi_rmint(0x10, &regs);
	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}

	*mi = *miptr;
	return 0;
}

int vid_setwin(int wid, int pos)
{
	struct dpmi_regs regs = {0};

	regs.eax = 0x4f05;
	regs.ebx = wid;
	regs.edx = pos;
	dpmi_rmint(0x10, &regs);
	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return 0;
}

static int conv_vbeinfo(int mode, struct vid_modeinfo *mi, struct vbe_mode_info *vbemi)
{
	static int gran_shift;
	static const struct { int width, height, bpp; } stdmode[] = {
		{640, 400, 8},		/* 100h */
		{640, 480, 8},		/* 101h */
		{800, 600, 4}, {800, 600, 8},		/* 102h - 103h */
		{1024, 768, 4}, {1024, 768, 8},		/* 104h - 105h */
		{1280, 1024, 4}, {1280, 1024, 8},	/* 106h - 107h */
		{80, 60, 4}, {132, 25, 4}, {132, 43, 4}, {132, 50, 4}, {132, 60, 4},
		{320, 200, 15}, {320, 200, 16}, {320, 200, 24},	/* 10dh - 10fh */
		{640, 480, 15}, {640, 480, 16}, {640, 480, 24},	/* 110h - 112h */
		{800, 600, 15}, {800, 600, 16}, {800, 600, 24},	/* 113h - 115h */
		{1024, 768, 15}, {1024, 768, 16}, {1024, 768, 24}, /* 116h - 118h */
		{1280, 1024, 15}, {1280, 1024, 16}, {1280, 1024, 24} /* 119h - 11bh */
	};

	if(!(vbemi->attr & VBE_ATTR_AVAIL)) {
		return -1;	/* ignore unsupported modes */
	}
	if(!(vbemi->attr & VBE_ATTR_GFX)) {
		return -1;	/* ignore text modes */
	}
	if(vbemi->attr & VBE_ATTR_LFB) {
		mi->lfb = 1;
	}

	mi->drv = &drv;
	mi->modeno = mode;
	mi->vmem_addr = 0xa0000;

	if(vbe_ver >= 0x0102) {
		mi->width = vbemi->xres;
		mi->height = vbemi->yres;
		mi->bpp = vbemi->bpp;
		mi->rshift = vbemi->rpos;
		mi->gshift = vbemi->gpos;
		mi->bshift = vbemi->bpos;
		mi->rmask = calc_mask(vbemi->rsize, vbemi->rpos);
		mi->gmask = calc_mask(vbemi->gsize, vbemi->gpos);
		mi->bmask = calc_mask(vbemi->bsize, vbemi->bpos);
		mi->pages = vbemi->num_img_pages + 1;

		if(vbe_ver >= 0x0200) {
			mi->vmem_addr = vbemi->fb_addr;
			mi->vmem_size = vbemi->scanline_bytes * mi->height * mi->pages;
		}
	} else {
		if((mode & 0xff) > 7) {
			return -1;
		}
		mi->width = stdmode[mode & 0xff].width;
		mi->height = stdmode[mode & 0xff].height;
		mi->bpp = stdmode[mode & 0xff].bpp;
	}
	mi->ncolors = 1 << mi->bpp;
	mi->pitch = vbemi->scanline_bytes;
	mi->win_size = vbemi->win_size;
	mi->win_gran = vbemi->win_gran;

	gran_shift = 0;
	mi->win_step = 1;
	if(mi->win_gran > 0 && mi->win_gran < 64) {
		int gran = mi->win_gran;
		while(gran < 64) {
			gran_shift++;
			gran <<= 1;
		}
		mi->win_step = 1 << gran_shift;
	}

	mi->ops.pack = pack;
	mi->ops.unpack = unpack;
	mi->ops.setpal = vga_setpal;
	mi->ops.getpal = vga_getpal;
	mi->ops.vsync = vid_vsync;
	mi->ops.clear = clear;
	mi->ops.blit = 0;
	mi->ops.blitfb = 0;
	mi->ops.flip = flip;
	return 0;
}

static unsigned int calc_mask(int nbits, int pos)
{
	int i;
	unsigned int mask = 0;

	for(i=0; i<nbits; i++) {
		mask = (mask << 1) | 1;
	}
	return mask << pos;
}

static void print_mode_info(int mode, struct vid_modeinfo *mi)
{
	printf("VBE mode %04x\n", mode);
	printf("  %dx%d %d bpp (%d colors)\n", mi->width, mi->height,
		   mi->bpp, mi->ncolors);
	printf("  pitch: %d bytes, %d vmem pages\n", mi->pitch, mi->pages);

	if(mi->bpp > 8) {
		printf("  RGB mask %06x %06x %06x (pos: %d %d %d)\n", (unsigned int)mi->rmask,
				(unsigned int)mi->gmask, (unsigned int)mi->bmask, mi->rshift,
				mi->gshift, mi->bshift);
	}

	if(mode & VBE_MODE_LFB) {
		printf("  LFB address %xh, size: %d\n", (unsigned int)mi->vmem_addr,
				(int)mi->vmem_size);
	} else {
		printf("  banked window %d kb, granularity: %d kb, step: %d\n", mi->win_size,
				mi->win_gran, mi->win_step);
	}
}


static void pack(uint32_t *pix, int r, int g, int b)
{
	*pix = (((uint32_t)r << cur_mi->rshift) & cur_mi->rmask) |
		(((uint32_t)g << cur_mi->gshift) & cur_mi->gmask) |
		(((uint32_t)b << cur_mi->bshift) & cur_mi->bmask);
}

static void unpack(uint32_t pix, int *r, int *g, int *b)
{
	*r = (pix & cur_mi->rmask) >> cur_mi->rshift;
	*g = (pix & cur_mi->gmask) >> cur_mi->gshift;
	*b = (pix & cur_mi->bmask) >> cur_mi->bshift;
}

static void clear(uint32_t color)
{
}

static void blit_lfb(int x, int y, int w, int h, void *fb, int pitch)
{
	int i, pixsz, spansz;
	unsigned char *dest, *src;

	/*dbgmsg("blit: %d,%d (%dx%d)\n", x, y, w, h);*/

	pixsz = (cur_mi->bpp + 7) >> 3;
	spansz = w * pixsz;

	dest = (char*)vid_vmem + cur_mi->pitch * y + x * pixsz;
	src = fb;

	for(i=0; i<h; i++) {
		memcpy(dest, src, spansz);
		dest += cur_mi->pitch;
		src += pitch;
	}
}

static void blit_banked(int x, int y, int w, int h, void *fb, int pitch)
{
	abort();
}

static void blitfb_lfb(void *fb, int pitch)
{
	int i, pixsz, spansz;
	unsigned char *dest, *src;

	pixsz = (cur_mi->bpp + 7) >> 3;
	spansz = cur_mi->width * pixsz;

	dest = vid_vmem;
	src = fb;

	for(i=0; i<cur_mi->height; i++) {
		memcpy(dest, src, spansz);
		dest += cur_mi->pitch;
		src += pitch;
	}
}

static void blitfb_banked(void *fb, int pitch)
{
	int sz, offs, pending, winsz;
	unsigned char *pptr = fb;

	winsz = cur_mi->win_size << 10;

	/* assume initial window offset at 0 */
	offs = 0;
	pending = cur_pgsize;
	while(pending > 0) {
		sz = pending > winsz ? winsz : pending;
		memcpy((void*)0xa0000, pptr, sz);
		pptr += sz;
		pending -= sz;
		offs += cur_mi->win_step;
		vid_setwin(0, offs);
	}
	vid_setwin(0, 0);
}

static void flip(int vsync)
{
	/* TODO */
}
