#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "gfx.h"
#include "screens.h"
#include "fonts.h"
#include "ui.h"
#include "imago2.h"

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
#ifndef MSDOS
	W_FULLSCR,
#endif
	W_VSYNC,
	W_VIEWDIST,

	W_VOL_MASTER,
	W_VOL_SFX,
	W_VOL_MUSIC,

	W_KEYMAP,
	W_JSMAP,
	W_JSCAL,

	W_BNBOX,

	NUM_WIDGETS
};

static void *widgets[NUM_WIDGETS];
static int ui_focus;

static uint16_t *bgpix;
static int bgwidth, bgheight;

#define VSEP	30
int options_init(void)
{
	struct ui_button *bn;
	struct ui_bnbox *bnbox;
	struct ui_list *list;
	struct ui_ckbox *ckbox;
	struct ui_slider *slider;
	int x = 300;
	int y = 80;

	if(!(bgpix = img_load_pixels("data/optbg.png", &bgwidth, &bgheight, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load options bg image\n");
	}

#ifndef MSDOS
	y -= 8;
#endif
	if(!(list = ui_list("Resolution"))) {
		return -1;
	}
	ui_move(list, x, y);
	populate_mode_list(list);
	widgets[W_RESLIST] = list;
	y += VSEP;

#ifndef MSDOS
	y -= 7;
	if(!(ckbox = ui_ckbox("Fullscreen", opt.fullscreen))) {
		return -1;
	}
	ui_move(ckbox, x, y);
	widgets[W_FULLSCR] = ckbox;
	y += VSEP - 7;
#endif

	if(!(ckbox = ui_ckbox("VSync", opt.vsync))) {
		return -1;
	}
	ui_move(ckbox, x, y);
	widgets[W_VSYNC] = ckbox;
	y += VSEP;
#ifndef MSDOS
	y -= 7;
#endif

	if(!(list = ui_list("View distance"))) {
		return -1;
	}
	ui_move(list, x, y);
	ui_list_append(list, "near", 0);
	ui_list_append(list, "medium", 0);
	ui_list_append(list, "far", 0);
	widgets[W_VIEWDIST] = list;
	y += VSEP;

	y += VSEP / 2;

	if(!(slider = ui_slider("Master", 0, 100))) {
		return -1;
	}
	ui_move(slider, x, y);
	ui_slider_set_step(slider, 10);
	widgets[W_VOL_MASTER] = slider;
	y += VSEP;

	if(!(slider = ui_slider("Sound FX", 0, 100))) {
		return -1;
	}
	ui_move(slider, x, y);
	ui_slider_set_step(slider, 10);
	widgets[W_VOL_SFX] = slider;
	y += VSEP;

	if(!(slider = ui_slider("Music", 0, 100))) {
		return -1;
	}
	ui_move(slider, x, y);
	ui_slider_set_step(slider, 10);
	widgets[W_VOL_MUSIC] = slider;
	y += VSEP;

	y += VSEP / 2;

	if(!(bn = ui_button("Key mapping"))) {
		return -1;
	}
	ui_move(bn, x, y);
	widgets[W_KEYMAP] = bn;
	y += VSEP;

	if(!(bn = ui_button("Joystick mapping"))) {
		return -1;
	}
	ui_move(bn, x, y);
	widgets[W_JSMAP] = bn;
	y += VSEP;

	if(!(bn = ui_button("Calibrate joystick"))) {
		return -1;
	}
	ui_move(bn, x, y);
	widgets[W_JSCAL] = bn;
	y += VSEP;

	y += VSEP;

	if(!(bnbox = ui_bnbox("Accept", "Cancel"))) {
		return -1;
	}
	ui_move(bnbox, 320, y);
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
	img_free_pixels(bgpix);
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

	if(opt.viewdist < 0) opt.viewdist = 0;
	if(opt.viewdist > 2) opt.viewdist = 2;
	ui_list_select(widgets[W_VIEWDIST], opt.viewdist);

#ifndef MSDOS
	ui_ckbox_set(widgets[W_FULLSCR], opt.fullscreen);
#endif
	ui_ckbox_set(widgets[W_VSYNC], opt.vsync);
	ui_slider_set(widgets[W_VOL_MASTER], (1000 * opt.vol_master + 500) / 2550);
	ui_slider_set(widgets[W_VOL_SFX], (1000 * opt.vol_sfx + 500) / 2550);
	ui_slider_set(widgets[W_VOL_MUSIC], (1000 * opt.vol_mus + 500) / 2550);

	set_focus(0);
}

void options_stop(void)
{
}

void options_draw(void)
{
	int i;

	if(bgpix) {
		memcpy(fb_pixels, bgpix, fb_size);
	} else {
		memset(fb_pixels, 0, fb_size);
	}

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

	default:
		ui_keypress(widgets[ui_focus], key);
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
		opt.bpp = mode->bpp == 32 ? 16 : mode->bpp;
	}

	opt.viewdist = ui_list_selection(widgets[W_VIEWDIST]);

#ifndef MSDOS
	opt.fullscreen = ui_ckbox_state(widgets[W_FULLSCR]);
	set_text_mode();
	set_video_mode(match_video_mode(640, 480, 16), 1);
#endif
	opt.vsync = ui_ckbox_state(widgets[W_VSYNC]);

	opt.vol_master = ui_slider_value(widgets[W_VOL_MASTER]) * 255 / 100;
	opt.vol_sfx = ui_slider_value(widgets[W_VOL_SFX]) * 255 / 100;
	opt.vol_mus = ui_slider_value(widgets[W_VOL_MUSIC]) * 255 / 100;

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
#ifdef WIN32
	static const int match_bpp_list[] = {16, 15, 32, 0};
#else
	static const int match_bpp_list[] = {16, 15, 0};
#endif
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
