#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "game.h"
#include "fonts.h"
#include "sprite.h"

static struct sprites fonts[NUM_FONTS];
static int fontsize[] = { 8, 16, 16, 32, 32 };

static const char *fontfiles[] = {
	"data/dbgfont.spr",
	"data/nebula.spr",
	"data/nebucol.spr",
	"data/nebcobig.spr",
	"data/nebcobhl.spr",
	0
};

static struct sprites *actfnt;
static int actsz;

static int align;

int init_fonts(void)
{
	int i;

	for(i=0; i<NUM_FONTS; i++) {
		if(load_sprites(fonts + i, fontfiles[i]) == -1) {
			fprintf(stderr, "failed to load sprite: %s\n", fontfiles[i]);
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

int selected_font(void)
{
	return actfnt - fonts;
}

void fnt_print(void *fb, int x, int y, const char *str)
{
	uint16_t *dest;

	switch(align) {
	case FONT_CENTER:
		x -= fnt_strwidth(str) / 2;
		break;
	case FONT_RIGHT:
		x -= fnt_strwidth(str);
		break;
	default:
		break;
	}

	dest = (uint16_t*)fb + y * fb_width + x;

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

void fnt_align(int a)
{
	align = a;
}

int fnt_strwidth(const char *str)
{
	return strlen(str) * actsz;
}

int fnt_width(int font)
{
	return fontsize[font];
}

int fnt_height(int font)
{
	return fontsize[font];
}
