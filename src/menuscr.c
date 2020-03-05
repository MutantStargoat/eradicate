#include <stdio.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"

static const struct menuent {
	int x, y, len, height;
} menuent[] = {
	{240, 300, 170, 32},
	{230, 360, 184, 32},
	{260, 424, 130, 32}
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
	offs = y * ent->len + ent->x;
	blit(blurbuf[0], ent->len, bgpix + offs, ent->len, ent->height, bgwidth);

	//blur_grey_horiz(blurbuf[1], blurbuf[0], ent->len, ent->height, 5, 0x100);
	for(i=0; i<ent->height; i++) {
		for(j=0; j<ent->len; j++) {
			blurbuf[1][i * ent->len + j] = 0xff;//~blurbuf[0][i * ent->len + j];
		}
	}

	wait_vsync();

	blit_frame(bgpix, 0);
	blit(fb_pixels + offs, fb_width, blurbuf[1], ent->len, ent->height, ent->len);
}

void menu_keyb(int key, int pressed)
{
	switch(key) {
	case 27:
		game_quit();
		break;
	}
}
