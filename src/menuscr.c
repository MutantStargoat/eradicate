#include <stdio.h>
#include <string.h>
#include <math.h>
#include "screens.h"
#include "imago2.h"
#include "gfx.h"
#include "gfxutil.h"
#include "game.h"
#include "util.h"
#include "3dgfx/3dgfx.h"
#include "3dgfx/mesh.h"
#include "joy.h"

#define PADX	42
#define PADX2	(PADX * 2)
#define PADY	16
#define PADY2	(PADY * 2)

static const struct menuent {
	int x, y, len, height;
} menuent[] = {
	{252 - PADX, 281 - PADY, 147 + PADX2, 37 + PADY2},
	{244 - PADX, 344 - PADY, 161 + PADX2, 37 + PADY2},
	{276 - PADX, 407 - PADY, 102 + PADX2, 38 + PADY2}
};

static int cur;

static uint16_t *bgpix;
static int bgwidth, bgheight;

static struct g3d_mesh logo_mesh;
static uint16_t *envpix;
static int envwidth, envheight;


int menu_init(void)
{
	if(!(bgpix = img_load_pixels("data/menbg640.png", &bgwidth, &bgheight, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load menu bg image\n");
		return -1;
	}

	if(!(envpix = img_load_pixels("data/refmap.jpg", &envwidth, &envheight, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load environment map\n");
		return -1;
	}

	if(load_mesh(&logo_mesh, "data/logo.obj") == -1) {
		fprintf(stderr, "failed to load logo mesh\n");
		return -1;
	}
	normalize_mesh_normals(&logo_mesh);
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

	g3d_reset();
	g3d_framebuffer(fb_width, fb_height, fb_pixels);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(60.0, (float)fb_width / (float)fb_height, 0.5, 100.0);

	g3d_enable(G3D_CULL_FACE);
	/*
	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);
	*/
	g3d_disable(G3D_CLIP_FRUSTUM);

	g3d_polygon_mode(G3D_FLAT);

	g3d_enable(G3D_TEXTURE_2D);
	g3d_enable(G3D_TEXTURE_GEN);
	g3d_set_texture(envwidth, envheight, envpix);

	g3d_matrix_mode(G3D_MODELVIEW);
}

void menu_stop(void)
{
}


#define BBW		256
#define BBH		64

void menu_draw(void)
{
	static uint16_t blurbuf[2][BBW * BBH];
	uint16_t *fb = fb_pixels;
	int fboffs, cleartop;
	int blur_rad_x, blur_rad_y;
	const struct menuent *ent;
	float sint, cost;

	sint = sin(time_msec / 1000.0f);
	cost = cos(time_msec / 1000.0f);

	/* simulate keyboard events from joystick input */
	if(have_joy) {
		joy_keyemu();
		if(draw != menu_draw) return;
	}

	memcpy(fb_pixels, bgpix, fb_size);
	g3d_load_identity();
	g3d_translate(0, 1.2, -4.5);
	g3d_rotate(cost * 4.0f, 1, 0, 0);
	g3d_rotate(sint * 6.0f, 0, 1, 0);

	/*zsort_mesh(&logo_mesh);*/
	draw_mesh(&logo_mesh);

	blur_rad_x = (int)((sint * 0.5f + 0.5f) * 50.0f);
	blur_rad_y = (int)((cost * 0.5f + 0.5f) * 50.0f);

	ent = menuent + cur;
	fboffs = ent->y * fb_width + ent->x;

	memset(blurbuf[0], 0, sizeof blurbuf[0]);
	blit(blurbuf[0], BBW, bgpix + fboffs, ent->len, ent->height, bgwidth);

	blur_horiz(blurbuf[1], blurbuf[0], BBW, BBH, blur_rad_x + 3, 0x140);
	blur_vert(blurbuf[0], blurbuf[1], BBW, BBH, blur_rad_y / 4 + 3, 0x140);

	//wait_vsync();

	cleartop = 280 * fb_width;
	memcpy(fb + cleartop, bgpix + cleartop, (fb_height - 280) * fb_width << 1);

	blit(fb + fboffs, fb_width, blurbuf[0], ent->len, ent->height, BBW);
	blit_key(fb + fboffs, fb_width, bgpix + fboffs, ent->len, ent->height, bgwidth, 0);

	if(show_fps) {
		blit(fb, fb_width, bgpix, 64, 16, bgwidth);
	}

	blit_frame(fb_pixels, 0);
}

void menu_keyb(int key, int pressed)
{
	if(!pressed) return;

	switch(key) {
#ifndef NDEBUG
	case 27:
		game_quit();
		break;
#endif

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
			race_start();
			break;

		case 1:
			options_start();
			break;

		case 2:
			game_quit();
			break;
		}
	}
}
