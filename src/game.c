#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "game.h"
#include "3dgfx/3dgfx.h"
#include "screens.h"
#include "fonts.h"
#include "joy.h"

#define GUARD_XPAD	0
#define GUARD_YPAD	1

int fb_width, fb_height, fb_bpp, fb_scan_size;
float fb_aspect;
long fb_size, fb_buf_size;
void *fb_pixels, *vmem;
void *fb_buf;

int have_joy;

long time_msec;
int show_fps = 1;

void (*draw)(void);
void (*key_event)(int key, int pressed);
void (*input_event)(int inp);


int init(int argc, char **argv)
{
	load_options(GAME_CFG_FILE);

	have_joy = joy_detect();

	if(init_fonts() == -1) {
		return -1;
	}

	if(g3d_init() == -1) {
		return -1;
	}

	if(intro_init() == -1) {
		return -1;
	}
	if(menu_init() == -1) {
		return -1;
	}
	if(options_init() == -1) {
		return -1;
	}
	if(race_init() == -1) {
		return -1;
	}

	intro_start();
	return 0;
}

void cleanup(void)
{
	g3d_destroy();
	race_cleanup();
	intro_cleanup();
	options_cleanup();
	menu_cleanup();
}

int resizefb(int width, int height, int bpp)
{
	int newsz, new_scansz;

	if(!width || !height || !bpp) {
		free(fb_buf);
		fb_buf = fb_pixels = 0;
		fb_size = fb_buf_size = fb_scan_size = 0;
		fb_width = fb_height = fb_bpp = 0;
		return 0;
	}

	new_scansz = ((width + GUARD_XPAD * 2) * bpp + 7) / 8;
	newsz = (height + GUARD_YPAD * 2) * new_scansz;

	if(!fb_buf || newsz > fb_buf_size) {
		void *tmp = malloc(newsz);
		if(!tmp) return -1;

		free(fb_buf);
		fb_buf = tmp;
		fb_buf_size = newsz;
	}

	fb_scan_size = new_scansz;
	fb_pixels = (char*)fb_buf + GUARD_YPAD * fb_scan_size + (GUARD_XPAD * bpp + 7) / 8;
	fb_width = width;
	fb_height = height;
	fb_bpp = bpp;
	fb_size = fb_scan_size * fb_height;

	fb_aspect = (float)fb_width / (float)fb_height;

	return 0;
}

void dbg_print(void *fb, int x, int y, const char *str)
{
	select_font(FONT_VGA);
	fnt_align(FONT_LEFT);
	fnt_print(fb, x, y, str);
}

void dbg_printf(void *fb, int x, int y, const char *fmt, ...)
{
	va_list ap;

	select_font(FONT_VGA);
	fnt_align(FONT_LEFT);
	va_start(ap, fmt);
	fnt_vprintf(fb, x, y, fmt, ap);
	va_end(ap);
}

void dbg_fps(void *fb)
{
	static char fpsbuf[16];
	static long frame, prev_upd;
	long msec = time_msec;
	long delta;

	frame++;

	delta = msec - prev_upd;
	if(delta >= 1024) {
		frame *= 1000;
		sprintf(fpsbuf, "%2ld.%1ld", frame >> 10, 10 * (frame & 0x3ff) >> 10);
		frame = 0;
		prev_upd = msec;
	}

	dbg_print(fb, 2, 2, fpsbuf);
}
