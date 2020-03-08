#include <stdio.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"

#define FADE_DUR	800

static void *logo;
static int logo_width, logo_height;
static long start_time;

int intro_init(void)
{
	if(!(logo = img_load_pixels("data/msglogo.jpg", &logo_width, &logo_height, IMG_FMT_RGB24))) {
		fprintf(stderr, "failed to load logo image\n");
		return -1;
	}
	return 0;
}

void intro_cleanup(void)
{
	img_free_pixels(logo);
}

void intro_start(void)
{
	draw = intro_draw;
	key_event = intro_keyb;

	start_time = time_msec;
}

void intro_stop(void)
{
}

void intro_draw(void)
{
	int i, j;
	long tm;
	uint16_t fade;
	unsigned char *src = logo;
	uint16_t *dest = fb_pixels;

	tm = time_msec - start_time;
	if(tm < FADE_DUR) {
		fade = tm * 256 / FADE_DUR;
	} else if(tm < FADE_DUR * 2) {
		fade = 256;
	} else if(tm < FADE_DUR * 3) {
		fade = 256 - (tm - 2 * FADE_DUR) * 256 / FADE_DUR;
	} else {
		fade = 0;
		//menu_start();
	}

	for(i=0; i<fb_height; i++) {
		for(j=0; j<fb_width; j++) {
			uint16_t r = (uint16_t)*src++ * fade / 256;
			uint16_t g = (uint16_t)*src++ * fade / 256;
			uint16_t b = (uint16_t)*src++ * fade / 256;
			*dest++ = PACK_RGB16(r, g, b);
		}
	}

	dbg_print(fb_pixels, 10, 10, "foo");

	blit_frame(fb_pixels, 1);
}

void intro_keyb(int key, int pressed)
{
	if(pressed) {
		menu_start();
	}
}
