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
#include "playlist.h"
#include "fonts.h"
#include "ui.h"

enum { MENU_START, MENU_OPTIONS, MENU_QUIT, NUM_MENU_ENTRIES};
static const char *menu_str[NUM_MENU_ENTRIES] = {"Start", "Options", "Quit"};
static void *menu_widget[NUM_MENU_ENTRIES];

static int cur;

static uint16_t *bgpix;
static int bgwidth, bgheight;

static struct g3d_mesh logo_mesh;
static uint16_t *envpix;
static int envwidth, envheight;

static struct playlist *mus;

int menu_init(void)
{
	int i;

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

	for(i=0; i<NUM_MENU_ENTRIES; i++) {
		struct ui_button *bn;
		if(!(bn = ui_button(menu_str[i]))) {
			return -1;
		}
		ui_move(bn, 50, 300 + i * 50);
		bn->w.font = FONT_MENU_SHADED_BIG;
		menu_widget[i] = bn;
	}
	ui_set_focus(menu_widget[0], 1);

	if((mus = create_playlist("data/musmenu"))) {
		shuffle_playlist(mus);
	}
	return 0;
}

void menu_cleanup(void)
{
	int i;
	for(i=0; i<NUM_MENU_ENTRIES; i++) {
		ui_free(menu_widget[i]);
	}

	if(mus) destroy_playlist(mus);
	img_free_pixels(bgpix);
}

void menu_start(void)
{
	printf("menu_start\n");
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

	if(mus && opt.music) {
		shuffle_playlist(mus);
		start_playlist(mus);
	}
}

void menu_stop(void)
{
}

void menu_draw(void)
{
	int i;
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

	for(i=0; i<NUM_MENU_ENTRIES; i++) {
		ui_draw(menu_widget[i]);
	}

	if(mus) proc_playlist(mus);

	if(show_fps) {
		blit(fb_pixels, fb_width, bgpix, 64, 16, bgwidth);
	}

	blit_frame(fb_pixels, opt.vsync);
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
		if(cur > 0) {
			ui_set_focus(menu_widget[cur], 0);
			ui_set_focus(menu_widget[--cur], 1);
		}
		break;

	case KB_DOWN:
		if(cur < NUM_MENU_ENTRIES - 1) {
			ui_set_focus(menu_widget[cur], 0);
			ui_set_focus(menu_widget[++cur], 1);
		}
		break;

	case '\n':
	case '\r':
		switch(cur) {
		case MENU_START:
			if(mus && opt.music) {
				stop_playlist(mus);
			}
			race_start();
			break;

		case MENU_OPTIONS:
			options_start();
			break;

		case MENU_QUIT:
			game_quit();
			break;
		}
		break;

	case '\t':
		if(mus && opt.music) {
			next_playlist(mus);
		}
		break;

	case 'm':
		if(mus) {
			if(opt.music) {
				cont_playlist(mus);
			} else {
				stop_playlist(mus);
			}
		}
		break;
	}
}
