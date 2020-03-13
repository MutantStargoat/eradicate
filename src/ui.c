#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ui.h"
#include "game.h"
#include "fonts.h"
#include "sprite.h"
#include "util.h"

static void free_bnbox(struct ui_bnbox *w);
static void free_list(struct ui_list *w);

static void draw_bnbox(struct ui_bnbox *w);
static void key_bnbox(struct ui_bnbox *w, int key);

static void draw_list(struct ui_list *w);
static void key_list(struct ui_list *w, int key);

static struct sprites spr_icons;


static void init_icons(void)
{
	static int done_init;
	if(done_init) return;

	load_sprites(&spr_icons, "data/icons.spr");
	done_init = 1;
}

static void init_base(struct ui_base *b)
{
	memset(b, 0, sizeof *b);

	init_icons();
}

static void free_base(struct ui_base *b)
{
	if(b) {
		free(b->text);
	}
}

struct ui_bnbox *ui_bnbox(const char *tx1, const char *tx2)
{
	int len1, len2;
	struct ui_bnbox *w;

	len1 = strlen(tx1);
	len2 = strlen(tx2);

	if(!(w = malloc(sizeof *w))) {
		perror("failed to allocate ui_bnbox");
		return 0;
	}
	init_base(&w->w);

	if(!(w->w.text = malloc(len1 + 1))) {
		perror("ui_bnbox: failed to allocate text buffer");
		free(w);
		return 0;
	}
	if(!(w->text2 = malloc(len2 + 1))) {
		perror("ui_bnbox: failed to allocate text buffer");
		free(w->w.text);
		free(w);
		return 0;
	}

	memcpy(w->w.text, tx1, len1 + 1);
	memcpy(w->text2, tx2, len2 + 1);

	w->sel = 0;

	w->w.draw = draw_bnbox;
	w->w.keypress = key_bnbox;
	w->w.free = free_bnbox;
	return w;
}

static void free_bnbox(struct ui_bnbox *w)
{
	if(w) {
		free_base(&w->w);
		free(w->text2);
		free(w);
	}
}

struct ui_ckbox *ui_ckbox(const char *text, int chk)
{
	return 0;
}

struct ui_list *ui_list(const char *text)
{
	struct ui_list *w;

	if(!(w = malloc(sizeof *w))) {
		perror("failed to allocate ui_list");
		return 0;
	}
	init_base(&w->w);
	if(ui_set_text(w, text) == -1) {
		free(w);
		return 0;
	}

	w->items = 0;
	w->num_items = w->max_items = 0;
	w->sel = -1;

	w->w.draw = draw_list;
	w->w.keypress = key_list;
	w->w.free = free_list;
	return w;
}

static void free_list(struct ui_list *w)
{
	int i;

	if(w) {
		free_base(&w->w);
		for(i=0; i<w->num_items; i++) {
			free(w->items[i].name);
		}
		free(w->items);
		free(w);
	}
}

void ui_free(void *w)
{
	struct ui_base *b = (struct ui_base*)w;
	b->free(w);
}

void ui_move(void *w, int x, int y)
{
	struct ui_base *b = (struct ui_base*)w;

	b->x = x;
	b->y = y;
}

int ui_set_text(void *w, const char *text)
{
	void *tmp;
	int len = strlen(text);
	struct ui_base *b = (struct ui_base*)w;

	if(!(tmp = malloc(len + 1))) {
		perror("ui_set_text: failed to allocate string buffer");
		return -1;
	}
	memcpy(tmp, text, len + 1);

	free(b->text);
	b->text = tmp;
	return 0;
}

void ui_set_focus(void *w, int focus)
{
	struct ui_base *b = (struct ui_base*)w;
	b->focus = focus;
}

void ui_draw(void *w)
{
	struct ui_base *b = (struct ui_base*)w;

	if(b->draw) {
		b->draw(w);
	}
}

void ui_keypress(void *w, int key)
{
	struct ui_base *b = (struct ui_base*)w;

	if(b->keypress) {
		b->keypress(w, key);
	}
}


void ui_bnbox_select(struct ui_bnbox *w, int sel)
{
	w->sel = sel ? 1 : 0;
}

void ui_bnbox_next(struct ui_list *w)
{
	if(w->sel < 1) w->sel++;
}

void ui_bnbox_prev(struct ui_list *w)
{
	if(w->sel > 0) w->sel--;
}


int ui_ckbox_state(struct ui_ckbox *w)
{
	return 0;
}

void ui_ckbox_set(struct ui_ckbox *w, int val)
{
}



int ui_list_append(struct ui_list *w, const char *name, void *udata)
{
	void *tmp;

	if(w->num_items >= w->max_items) {
		int newmax = w->max_items ? w->max_items * 2 : 8;
		if(!(tmp = realloc(w->items, newmax * sizeof *w->items))) {
			perror("ui_list_append: failed to resize items array");
			return -1;
		}
		w->items = tmp;
		w->max_items = newmax;
	}

	w->items[w->num_items].name = strdup(name);
	w->items[w->num_items++].data = udata;

	if(w->sel == -1) w->sel = 0;
	return 0;
}

void ui_list_select(struct ui_list *w, int sel)
{
	if(sel < 0 || sel >= w->num_items) sel = -1;
	w->sel = sel;
}

void ui_list_next(struct ui_list *w)
{
	if(w->sel < w->num_items - 1) {
		w->sel++;
	}
}

void ui_list_prev(struct ui_list *w)
{
	if(w->sel > 0) {
		w->sel--;
	}
}

static void draw_selbox(int x, int y, int w, int h)
{
	float t = (float)time_msec / 100.0f;
	double d = (sin(t) * 0.5 + 0.5) * 8.0;
	int dist = cround64(d);
	int x0, y0;
	unsigned char *fb = fb_pixels;

	x0 = x - dist - 16;
	y0 = y + h / 2 - 8;
	draw_sprite(fb + y0 * fb_scan_size + (x0 << 1), fb_scan_size, &spr_icons, 0);
}

static void draw_bnbox(struct ui_bnbox *w)
{
	int x, y;
	int width1, width2;

	select_font(FONT_MENU);
	fnt_align(FONT_LEFT);

	width1 = fnt_strwidth(w->w.text);
	width2 = fnt_strwidth(w->text2);

	x = w->w.x - width1 - 30;
	y = w->w.y;
	if(w->w.focus && w->sel == 0) draw_selbox(x - 3, y, width1, 16);
	fnt_print(fb_pixels, x, y, w->w.text);

	x = w->w.x + 30;
	fnt_print(fb_pixels, x, y, w->text2);
	if(w->w.focus && w->sel == 1) draw_selbox(x - 3, y, width2, 16);
}

static void key_bnbox(struct ui_bnbox *w, int key)
{
	switch(key) {
	case KB_LEFT:
		if(w->sel > 0) w->sel--;
		break;
	case KB_RIGHT:
		if(w->sel < 1) w->sel++;
		break;
	}
}


static void draw_list(struct ui_list *w)
{
	int i, x, y, seldist;
	uint16_t *fbptr;

	select_font(FONT_MENU_SHADED);
	fnt_align(FONT_RIGHT);

	x = w->w.x - 16;
	y = w->w.y;

	fnt_print(fb_pixels, x, y, w->w.text);
	x += 32;

	select_font(FONT_MENU);
	fnt_align(FONT_LEFT);

	fbptr = (uint16_t*)((unsigned char*)fb_pixels + y * fb_scan_size + x * fb_bpp / 8);

	for(i=0; i<w->num_items; i++) {
		seldist = w->sel >= 0 ? abs(i - w->sel) : 100;
		if(seldist > 3) seldist = 3;

		if(seldist) {
			draw_sprite(fbptr, fb_scan_size, &spr_icons, seldist + 1);
			fbptr += 16;
		} else {
			fnt_print(fbptr, 0, 0, w->items[i].name);
			fbptr += fnt_strwidth(w->items[i].name);
		}
	}
}

static void key_list(struct ui_list *w, int key)
{
	switch(key) {
	case KB_LEFT:
		ui_list_prev(w);
		break;

	case KB_RIGHT:
		ui_list_next(w);
		break;
	}
}
