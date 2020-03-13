#include <string.h>
#include "game.h"
#include "gfx.h"
#include "screens.h"
#include "fonts.h"
#include "ui.h"

#define NUM_WIDGETS	2
static void *widgets[NUM_WIDGETS];
static int ui_focus;

int options_init(void)
{
	struct ui_bnbox *bnbox;
	struct ui_list *list;

	if(!(list = ui_list("Resolution"))) {
		return -1;
	}
	ui_move(list, 320, 100);
	ui_list_append(list, "320x240", 0);
	ui_list_append(list, "400x300", 0);
	ui_list_append(list, "512x384", 0);
	ui_list_append(list, "640x480", 0);
	ui_list_append(list, "800x600", 0);
	ui_list_append(list, "1024x768", 0);
	ui_list_append(list, "1280x1024", 0);
	widgets[0] = list;

	if(!(bnbox = ui_bnbox("Done", "Cancel"))) {
		return -1;
	}
	ui_move(bnbox, 320, 250);
	widgets[1] = bnbox;

	ui_set_focus(widgets[ui_focus], 1);

	return 0;
}

void options_cleanup(void)
{
	int i;
	for(i=0; i<NUM_WIDGETS; i++) {
		ui_free(widgets[i]);
	}
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
	int i;

	memset(fb_pixels, 0, fb_size);

	select_font(FONT_MENU_SHADED_BIG);
	fnt_align(FONT_CENTER);
	fnt_print(fb_pixels, 320, 20, "Options");

	for(i=0; i<NUM_WIDGETS; i++) {
		ui_draw(widgets[i]);
	}

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

	case KB_UP:
		if(ui_focus > 0) {
			ui_set_focus(widgets[ui_focus], 0);
			ui_set_focus(widgets[--ui_focus], 1);
		}
		break;

	case KB_DOWN:
		if(ui_focus < NUM_WIDGETS - 1) {
			ui_set_focus(widgets[ui_focus], 0);
			ui_set_focus(widgets[++ui_focus], 1);
		}
		break;

	case KB_LEFT:
	case KB_RIGHT:
		ui_keypress(widgets[ui_focus], key);
		break;

	case '\n':
	case '\r':
		/* TODO */
		break;
	}
}
