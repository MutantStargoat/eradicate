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
	fnt_print(fb_pixels, 200, 20, "Options");

	select_font(FONT_MENU_SHADED);
	fnt_print(fb_pixels, 160, 100, "Resolution");

	select_font(FONT_MENU);
	fnt_print(fb_pixels, 350, 100, "640x480");

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
