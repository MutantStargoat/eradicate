#include <stdio.h>
#include <limits.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"
#include "joy.h"

#define FADE_DUR	800

static void *logo;
static int logo_width, logo_height;
static long start_time;

int intro_init(void)
{
	if(!(logo = img_load_pixels("data/msglogo.jpg", &logo_width, &logo_height, IMG_FMT_BGRA32))) {
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

	start_time = LONG_MIN;
}

void intro_stop(void)
{
}

void fade_image(void *dest, void *src, uint16_t fade);

void intro_draw(void)
{
	long tm;
	uint16_t fade;

	if(have_joy) {
		if(joy_bnpress & JOY_BN_ANY) {
			menu_start();
			return;
		}
	}

	if(start_time == LONG_MIN) {
		start_time = time_msec;
	}

	tm = time_msec - start_time;
	if(tm < FADE_DUR) {
		fade = tm * 256 / FADE_DUR;
	} else if(tm < FADE_DUR * 2) {
		fade = 256;
	} else if(tm < FADE_DUR * 3) {
		fade = 256 - (tm - 2 * FADE_DUR) * 256 / FADE_DUR;
	} else {
		fade = 0;
		menu_start();
		return;
	}

	{
		int i;
		uint32_t *src = logo;
		uint16_t *dest = fb_pixels;
		for(i=0; i<640*480; i++) {
			uint32_t pix = *src++;
			uint16_t r = (uint16_t)UNPACK_R32(pix) * fade / 256;
			uint16_t g = (uint16_t)UNPACK_G32(pix) * fade / 256;
			uint16_t b = (uint16_t)UNPACK_B32(pix) * fade / 256;
			*dest++ = PACK_RGB16(r, g, b);
		}
	}

	blit_frame(fb_pixels, 0);
}

void intro_keyb(int key, int pressed)
{
	if(pressed) {
		menu_start();
	}
}
