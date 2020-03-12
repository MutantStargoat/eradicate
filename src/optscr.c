#include <string.h>
#include "game.h"
#include "gfx.h"
#include "screens.h"
#include "fonts.h"


int options_init(void)
{
	return 0;
}

void options_cleanup(void)
{
}

void options_start(void)
{
	draw = options_draw;
	key_event = options_keyb;
}

void options_stop(void)
{
}

void options_draw(void)
{
	memset(fb_pixels, 0, fb_size);

	select_font(FONT_MENU_SHADED_BIG);
	fnt_align(FONT_CENTER);
	fnt_print(fb_pixels, 320, 20, "Options");

	select_font(FONT_MENU_SHADED);
	fnt_align(FONT_RIGHT);
	fnt_print(fb_pixels, 320, 100, "Resolution");

	select_font(FONT_MENU);
	fnt_align(FONT_LEFT);
	fnt_print(fb_pixels, 360, 100, "640x480");

	blit_frame(fb_pixels, 0);
}

void options_keyb(int key, int pressed)
{
	if(!pressed) return;

	switch(key) {
	case 27:
		options_stop();
		menu_start();
		break;

	case '\n':
	case '\r':
		/* TODO */
		break;
	}
}
