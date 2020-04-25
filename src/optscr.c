#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "gfx.h"
#include "screens.h"
#include "fonts.h"
#include "ui.h"

struct mode_item {
	int idx;
	int width, height, bpp;
};
struct mode_item *modelist;
int modelist_size;

static void set_focus(int widx);
static void apply_options(void);
static int populate_mode_list(struct ui_list *widget);

enum {
	W_RESLIST,
	W_BNBOX,

	NUM_WIDGETS
};

static void *widgets[NUM_WIDGETS];
static int ui_focus;

int options_init(void)
{
	struct ui_bnbox *bnbox;
	struct ui_list *list;

	if(!(list = ui_list("Resolution"))) {
		return -1;
	}
	ui_move(list, 300, 100);
	populate_mode_list(list);
	widgets[W_RESLIST] = list;

	if(!(bnbox = ui_bnbox("Accept", "Cancel"))) {
		return -1;
	}
	ui_move(bnbox, 320, 250);
	widgets[W_BNBOX] = bnbox;

	ui_set_focus(widgets[ui_focus], 1);

	return 0;
}

void options_cleanup(void)
{
	int i;
	for(i=0; i<NUM_WIDGETS; i++) {
		ui_free(widgets[i]);
	}
	free(modelist);
}

void options_start(void)
{
	int i;
	struct ui_list *wlist;

	draw = options_draw;
	key_event = options_keyb;

	wlist = widgets[W_RESLIST];
	for(i=0; i<wlist->num_items; i++) {
		struct mode_item *m = wlist->items[i].data;
		if(m->width == opt.xres && m->height == opt.yres) {
			ui_list_select(wlist, i);
		}
	}

	set_focus(0);
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

	blit_frame(fb_pixels, opt.vsync);
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
			set_focus(ui_focus - 1);
		}
		break;

	case KB_DOWN:
		if(ui_focus < NUM_WIDGETS - 1) {
			set_focus(ui_focus + 1);
		}
		break;

	case KB_LEFT:
	case KB_RIGHT:
		ui_keypress(widgets[ui_focus], key);
		break;

	case '\n':
	case '\r':
	case ' ':
		if(ui_focus == W_BNBOX) {
			if(ui_bnbox_getsel(widgets[ui_focus]) == 0) {
				apply_options();
			}
			options_stop();
			menu_start();
		}
		break;
	}
}

static void set_focus(int widx)
{
	if(ui_focus != widx) {
		ui_set_focus(widgets[ui_focus], 0);
		ui_focus = widx;
		ui_set_focus(widgets[ui_focus], 1);
	}
}

static void apply_options(void)
{
	struct mode_item *mode = ui_list_sel_data(widgets[W_RESLIST]);
	if(mode) {
		opt.xres = mode->width;
		opt.yres = mode->height;
		opt.bpp = mode->bpp;
	}

	save_options("game.cfg");
}

static int modecmp(const void *a, const void *b)
{
	const struct mode_item *ma = a;
	const struct mode_item *mb = b;

	uint32_t sa = (ma->height << 16) | ma->width;
	uint32_t sb = (mb->height << 16) | mb->width;

	return sa - sb;
}

static int populate_mode_list(struct ui_list *widget)
{
	static const int match_bpp_list[] = {16, 15, 0};
	struct mode_item *item;
	struct video_mode *modes;
	int i, j, num_modes;
	char resname[32];

	modes = video_modes();
	num_modes = num_video_modes();

	if(!modes || !num_modes) return -1;

	if(!(modelist = malloc(num_modes * sizeof *modelist))) {
		perror("failed to allocate mode list");
		return -1;
	}
	modelist_size = 0;

	for(i=0; match_bpp_list[i]; i++) {
		for(j=0; j<num_modes; j++) {
			if(modes[j].bpp == match_bpp_list[i]) {
				item = modelist + modelist_size++;
				item->idx = j;
				item->width = modes[j].xsz;
				item->height = modes[j].ysz;
				item->bpp = modes[j].bpp;
			}
		}
		if(modelist_size) break;
	}

	if(!modelist_size) return -1;

	qsort(modelist, modelist_size, sizeof *modelist, modecmp);

	for(i=0; i<modelist_size; i++) {
		sprintf(resname, "%dx%d", modelist[i].width, modelist[i].height);
		ui_list_append(widget, resname, modelist + i);
	}

	return 0;
}
