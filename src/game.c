#include <stdio.h>
#include <stdarg.h>
#include "game.h"
#include "screens.h"
#include "sprite.h"

int fb_width, fb_height;
long fb_size;
uint16_t *fb_pixels;

long time_msec;

void (*draw)(void);
void (*key_event)(int key, int pressed);

static struct sprites dbgfont;


int init(int argc, char **argv)
{
	if(load_sprites(&dbgfont, "data/dbgfont.spr") == -1) {
		return -1;
	}
	if(intro_init() == -1) {
		return -1;
	}
	if(menu_init() == -1) {
		return -1;
	}

	intro_start();
	return 0;
}

void cleanup(void)
{
	intro_cleanup();
	menu_cleanup();
}

void dbg_print(void *fb, int x, int y, const char *str)
{
	uint16_t *dest = (uint16_t*)fb + y * fb_width + x;

	while(*str) {
		int c = *str++;

		if(c > ' ' && c < 128) {
			draw_sprite(dest, fb_width * 2, &dbgfont, c - ' ');
		}
		dest += 8;
	}
}

void dbg_printf(void *fb, int x, int y, const char *fmt, ...)
{
	static char buf[2048];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	dbg_print(fb, x, y, buf);
}

void dbg_fps(void *fb)
{
	static char fpsbuf[8];
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
