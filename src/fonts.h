#ifndef FONTS_H_
#define FONTS_H_

#include <stdarg.h>

enum {
	FONT_VGA,
	FONT_MENU,
	FONT_MENU_SHADED,
	FONT_MENU_SHADED_BIG,

	NUM_FONTS
};

/* arguments to fnt_align */
enum {
	FONT_LEFT,
	FONT_CENTER,
	FONT_RIGHT
};

int init_fonts(void);
void select_font(int idx);

void fnt_print(void *fb, int x, int y, const char *str);
void fnt_printf(void *fb, int x, int y, const char *fmt, ...);
void fnt_vprintf(void *fb, int x, int y, const char *fmt, va_list ap);

void fnt_align(int align);
int fnt_strwidth(const char *str);

#endif	/* FONTS_H_ */
