#include <stdio.h>
#include <stdarg.h>
#include "game.h"
#include "fonts.h"
#include "sprite.h"

static struct sprites fonts[NUM_FONTS];
static int fontsize[] = { 8, 16, 16, 32 };

static const char *fontfiles[] = {
	"data/dbgfont.spr",
	"data/nebula.spr",
	"data/nebucol.spr",
	"data/nebcobig.spr",
	0
};

static struct sprites *actfnt;
static int actsz;

int init_fonts(void)
{
	int i;

	for(i=0; i<NUM_FONTS; i++) {
		if(load_sprites(fonts + i, fontfiles[i]) == -1) {
			return -1;
		}
	}
	return 0;
}

void select_font(int idx)
{
	actfnt = fonts + idx;
	actsz = fontsize[idx];
}

void fnt_print(void *fb, int x, int y, const char *str)
{
	uint16_t *dest = (uint16_t*)fb + y * fb_width + x;

	while(*str) {
		int c = *str++;

		if(c > ' ' && c < 128) {
			draw_sprite(dest, fb_width * 2, actfnt, c - ' ');
		}
		dest += actsz;
	}
}

void fnt_printf(void *fb, int x, int y, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fnt_vprintf(fb, x, y, fmt, ap);
	va_end(ap);
}

void fnt_vprintf(void *fb, int x, int y, const char *fmt, va_list ap)
{
	static char buf[2048];
	vsprintf(buf, fmt, ap);

	fnt_print(fb, x, y, buf);
}