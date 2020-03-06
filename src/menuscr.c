#include <stdio.h>
#include <string.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"

static const struct menuent {
	int x, y, len, height;
} menuent[] = {
	{240, 300, 170, 40},
	{230, 360, 184, 40},
	{260, 424, 130, 40}
};

static int cur;

static uint16_t *bgpix;
static int bgwidth, bgheight;

int menu_init(void)
{
	if(!(bgpix = img_load_pixels("data/menbg640.png", &bgwidth, &bgheight, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load menu bg image\n");
		return -1;
	}
	return 0;
}

void menu_cleanup(void)
{
	img_free_pixels(bgpix);
}

void menu_start(void)
{
	draw = menu_draw;
	key_event = menu_keyb;
}

void menu_stop(void)
{
}



void menu_draw(void)
{
	static uint16_t blurbuf[2][16384];
	int y, offs;
	int i, j;
	const struct menuent *ent = menuent + cur;

	y = ent->y - ent->height / 2;
	offs = y * fb_width + ent->x;
	blit(blurbuf[0], ent->len, bgpix + offs, ent->len, ent->height, bgwidth);

	blur_grey_horiz(blurbuf[1], blurbuf[0], ent->len, ent->height, 7, 0x100);

	wait_vsync();

	memcpy(fb_pixels, bgpix, fb_size);
	blit(fb_pixels + offs, fb_width, blurbuf[1], ent->len, ent->height, ent->len);

	blit_frame(fb_pixels, 0);
}

void menu_keyb(int key, int pressed)
{
	if(!pressed) return;

	switch(key) {
	case 27:
		game_quit();
		break;

	case KB_UP:
		if(cur > 0) cur--;
		break;

	case KB_DOWN:
		if(cur < sizeof menuent / sizeof *menuent - 1) {
			cur++;
		}
		break;
	}
}
