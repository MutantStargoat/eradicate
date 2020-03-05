#include <stdio.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"

static void *bgpix;
static int bgwidth, bgheight;

int menu_init(void)
{
	if(!(bgpix = img_load_pixels("data/menbg640.png", &bgwidth, &bgheight, IMG_FMT_RGB24))) {
		fprintf(stderr, "failed to load menu bg image\n");
		return -1;
	}
	convimg_rgb24_rgb16(bgpix, bgpix, bgwidth, bgheight);
	return 0;
}

void menu_cleanup(void)
{
	img_free_pixels(bgpix);
}

void menu_start(void)
{
}

void menu_stop(void)
{
}

void menu_draw(void)
{
	blit_frame(bgpix, 1);
}

void menu_keyb(int key, int pressed)
{
	switch(key) {
	case 27:
		game_quit();
		break;
	}
}
