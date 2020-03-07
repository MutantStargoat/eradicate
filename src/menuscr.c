#include <stdio.h>
#include <string.h>
#include <math.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"

static const struct menuent {
	int x, y, len, height;
} menuent[] = {
	{240, 300, 170, 48},
	{230, 360, 184, 48},
	{260, 424, 130, 48}
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


#define BBW		256
#define BBH		64

void menu_draw(void)
{
	static uint16_t blurbuf[2][BBW * BBH];
	int fboffs, bboffs, tmp;
	const struct menuent *ent = menuent + cur;

	int blur_rad_x = (int)((sin(time_msec / 1000.0f) * 0.5f + 0.5f) * 50.0f);
	int blur_rad_y = (int)((cos(time_msec / 1000.0f) * 0.5f + 0.5f) * 50.0f);

	fboffs = (ent->y - ent->height / 2) * fb_width + ent->x;
	bboffs = (BBH - ent->height) / 2 * BBW + BBW / 2;

	memset(blurbuf[0], 0, sizeof blurbuf[0]);
	blit(blurbuf[0] + bboffs, BBW, bgpix + fboffs, ent->len, ent->height, bgwidth);

	blur_horiz(blurbuf[1], blurbuf[0], BBW, BBH, blur_rad_x + 3, 0x140);
	blur_vert(blurbuf[0], blurbuf[1], BBW, BBH, blur_rad_y / 4 + 3, 0x140);

	wait_vsync();

	memcpy(fb_pixels, bgpix, fb_size);
	tmp = fboffs;
	fboffs -= 8 * fb_width + 32;
	bboffs -= 8 * BBW + 32;
	blit(fb_pixels + fboffs, fb_width, blurbuf[0] + bboffs, ent->len + 64, ent->height + 16, BBW);
	fboffs = tmp;
	blit_key(fb_pixels + fboffs, fb_width, bgpix + fboffs, ent->len, ent->height, bgwidth, 0);

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

	case '\n':
	case '\r':
		switch(cur) {
		case 0:
			/* enter game */
			break;

		case 1:
			//options_start();
			break;

		case 2:
			game_quit();
			break;
		}
	}
}
