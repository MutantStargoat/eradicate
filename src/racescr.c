#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screens.h"
#include "game.h"
#include "gfx.h"
#include "3dgfx/3dgfx.h"
#include "3dgfx/mesh.h"

static struct g3d_mesh mesh_ship;

int race_init(void)
{
	if(load_mesh(&mesh_ship, "data/ship.obj") == -1) {
		fprintf(stderr, "failed to load ship mesh\n");
		return -1;
	}
	return 0;
}

void race_cleanup(void)
{
	destroy_mesh(&mesh_ship);
}

void race_start(void)
{
	draw = race_draw;
	key_event = race_keyb;

	/* TODO save menu video mode, and switch to game video mode */

	g3d_framebuffer(fb_width, fb_height, fb_pixels);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(50.0, (float)fb_width / (float)fb_height, 0.5, 100.0);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);
}

void race_stop(void)
{
	/* TODO switch back to menu video mode */
}

void race_draw(void)
{
	memset(fb_pixels, 0, fb_size);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();
	g3d_translate(0, 0, -10);
	g3d_rotate(15, 1, 0, 0);
	g3d_translate(0, -2, 0);

	zsort_mesh(&mesh_ship);
	draw_mesh(&mesh_ship);

	blit_frame(fb_pixels, 0);
}

void race_keyb(int key, int pressed)
{
	if(!pressed) return;

	switch(key) {
	case 27:
		race_stop();
		menu_start();
		break;

	default:
		break;
	}
}
