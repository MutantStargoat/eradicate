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


static void init_base(struct ui_base *b)
{
	memset(b, 0, sizeof *b);
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
	return 0;
}

static void free_list(struct ui_list *w)
{
	int i;

	if(w) {
		free_base(&w->w);
		for(i=0; i<w->num_items; i++) {
			free(w->item[i]);
		}
		free(w->item);
		free(w->data);
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

int ui_ckbox_state(struct ui_ckbox *w)
{
	return 0;
}

void ui_ckbox_set(struct ui_ckbox *w, int val)
{
}


void ui_bnbox_select(struct ui_bnbox *w, int sel)
{
}

void ui_bnbox_next(struct ui_list *w)
{
}

void ui_bnbox_prev(struct ui_list *w)
{
}


int ui_list_append(struct ui_list *w, const char *name, void *udata)
{
	return 0;
}

void ui_list_select(struct ui_list *w, int sel)
{
}

void ui_list_next(struct ui_list *w)
{
}

void ui_list_prev(struct ui_list *w)
{
}

#define SKIP(len)		{SOP_SKIP, (len) << 1, 0}
#define FILL(len, x)	{SOP_FILL, (len) << 1, (void*)(x)}
#define ENDL			{SOP_ENDL, 0, 0}
#define ENDSPRITE		{SOP_END, 0, 0}

#define COL		0xffff

static struct sprite_op spr_op_rarrow[] = {
	FILL(2, COL), ENDL,
	FILL(1, COL), SKIP(1), FILL(2, COL), ENDL,
	SKIP(1), FILL(1, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(1), FILL(1, COL), SKIP(1), FILL(1, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(2), FILL(1, COL), SKIP(1), FILL(2, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(2), FILL(1, COL), SKIP(1), FILL(4, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(3), FILL(1, COL), SKIP(1), FILL(5, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(3), FILL(1, COL), SKIP(1), FILL(7, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(3), FILL(1, COL), SKIP(1), FILL(5, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(2), FILL(1, COL), SKIP(1), FILL(4, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(2), FILL(1, COL), SKIP(1), FILL(2, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(1), FILL(1, COL), SKIP(1), FILL(1, COL), SKIP(2), FILL(2, COL), ENDL,
	SKIP(1), FILL(1, COL), SKIP(2), FILL(2, COL), ENDL,
	FILL(1, COL), SKIP(1), FILL(2, COL), ENDL,
	FILL(2, COL), ENDL,
	ENDSPRITE
};

static struct sprite shape_sprites[] = {
	{spr_op_rarrow, sizeof spr_op_rarrow / sizeof *spr_op_rarrow}
};

static struct sprites spr_shapes = {
	16, 16, 16,		/* width, height, bpp */
	shape_sprites, 1
};

static void draw_selbox(int x, int y, int w, int h)
{
	float t = (float)time_msec / 100.0f;
	double d = (sin(t) * 0.5 + 0.5) * 8.0;
	int dist = cround64(d);
	int x0, y0;
	unsigned char *fb = fb_pixels;

	x0 = x - dist - 16;
	y0 = y + h / 2 - 8;
	draw_sprite(fb + y0 * fb_scan_size + (x0 << 1), fb_scan_size, &spr_shapes, 0);
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
	fnt_print(fb_pixels, x, y, w->w.text);
	if(w->w.focus && w->sel == 0) draw_selbox(x - 3, y, width1, 16);

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
