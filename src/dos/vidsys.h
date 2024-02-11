#ifndef VIDSYS_VIDEO_H_
#define VIDSYS_VIDEO_H_

#include <stdlib.h>
#include "inttypes.h"

struct vid_drvops;

struct vid_color {
	int r, g, b;
};

struct vid_gfxops {
	void (*pack)(uint32_t *pix, int r, int g, int b);
	void (*unpack)(uint32_t pix, int *r, int *g, int *b);
	void (*setpal)(int idx, int count, const struct vid_color *col);
	void (*getpal)(int idx, int count, struct vid_color *col);
	void (*vsync)(void);

	void (*clear)(uint32_t color);
	void (*blitfb)(void *fb, int pitch);
	void (*flip)(int vsync);

	void (*fill)(int x, int y, int w, int h, uint32_t color);
	void (*blit)(int x, int y, int w, int h, void *img, int pitch);
	void (*line)(int x0, int y0, int x1, int y1, uint32_t color);
	void (*hline)(int x, int y, int len, uint32_t color);
	void (*vline)(int x, int y, int len, uint32_t color);
};

struct vid_driver {
	const char *name;
	int prio;

	struct vid_modeinfo *modes;
	int num_modes;

	struct vid_drvops *ops;
};

struct vid_modeinfo {
	int modeno;
	int width, height, bpp, pitch;
	int ncolors;
	uint32_t rmask, gmask, bmask;
	int rshift, gshift, bshift;
	int pages;
	int win_size, win_gran, win_step;
	uint32_t vmem_addr;
	size_t vmem_size;
	int lfb;

	struct vid_driver *drv;
	struct vid_gfxops ops;
};

int vid_init(void);
void vid_cleanup(void);

int vid_curmode(void);
void *vid_setmode(int mode);
struct vid_modeinfo **vid_modes(void);
int vid_num_modes(void);
int vid_findmode(int xsz, int ysz, int bpp);
struct vid_modeinfo *vid_modeinfo(int mode);

void vid_vsync(void);				/* defined in drv_vga.c */
int vid_setwin(int win, int pos);	/* defined in drv_vbe.c */

/* current mode functions */
int vid_islinear(void);
int vid_isbanked(void);

void vid_setpal(int idx, int count, const struct vid_color *col);
void vid_getpal(int idx, int count, struct vid_color *col);

void vid_blit(int x, int y, int w, int h, void *src, int pitch);
void vid_blitfb(void *fb, int pitch);
void vid_blit32(int x, int y, int w, int h, uint32_t *src, int pitch);
void vid_blitfb32(uint32_t *fb, int pitch);

#endif	/* VIDSYS_VIDEO_H_ */
