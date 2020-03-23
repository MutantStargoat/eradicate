#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screens.h"
#include "game.h"
#include "gfx.h"
#include "3dgfx/3dgfx.h"
#include "3dgfx/mesh.h"
#include "imago2.h"
#include "track.h"
#include "fonts.h"
#include "camera.h"

enum {
	INP_FWD,
	INP_BRK,
	INP_LTURN,
	INP_RTURN,
	INP_LBRK,
	INP_RBRK,
	INP_FIRE,
	INP_CAM,
	NUM_INPUTS
};

static int keymap[NUM_INPUTS][2] = {
	{'w', KB_UP},
	{'s', KB_DOWN},
	{'a', KB_LEFT},
	{'d', KB_RIGHT},
	{'q', 'z'},
	{'e', 'x'},
	{' ', KB_NUM_0},
	{'\t', -1}
};

static int inpstate[NUM_INPUTS];


#define TRK_SUBDIV	26
#define TRK_TWIST	30

static struct g3d_mesh ship_mesh;
static uint16_t *ship_tex;
static int ship_tex_width, ship_tex_height;

static uint16_t *road_tex;
static int road_tex_width, road_tex_height;

static int menu_mode_idx = -1;

static struct curve *path;
static struct track trk;
static struct camera cam[2];
static int act_cam;

static cgm_vec3 ppos, pdir, pvel;
static float pspeed;
static float pxform[16];
static float projt;
static cgm_vec3 proj_pos;

static long prev_upd;

int race_init(void)
{
	if(load_mesh(&ship_mesh, "data/ship.obj") == -1) {
		fprintf(stderr, "failed to load ship mesh\n");
		return -1;
	}
	if(!(ship_tex = img_load_pixels("data/shiptex.png", &ship_tex_width,
				&ship_tex_height, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load ship texture\n");
		return -1;
	}
	if(!(road_tex = img_load_pixels("data/road.png", &road_tex_width, &road_tex_height,
				IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load road texture\n");
		return -1;
	}

	return 0;
}

void race_cleanup(void)
{
	img_free_pixels(ship_tex);
	img_free_pixels(road_tex);
	destroy_mesh(&ship_mesh);
}

void race_start(void)
{
	int vmidx;

	memset(fb_pixels, 0, fb_size);
	select_font(FONT_MENU_SHADED);
	fnt_align(FONT_CENTER);
	fnt_print(fb_pixels, 320, 220, "Loading track...");
	blit_frame(fb_pixels, 0);

	if(!(path = load_curve("data/track1.trk"))) {
		fprintf(stderr, "failed to load track path\n");
		return;
	}
	path->mode = CURVE_REPEAT;
	path->proj_refine_thres = 1e-5;

	if(create_track(&trk, path) == -1) {
		fprintf(stderr, "failed to load track\n");
		return;
	}
	if(gen_track_mesh(&trk, TRK_SUBDIV, TRK_TWIST) == -1) {
		fprintf(stderr, "failed to generate track mesh\n");
		destroy_track(&trk);
		return;
	}

	pspeed = 0;
	projt = 0;
	eval_curve(path, 0, &ppos);
	eval_tangent(path, 0, &pdir);
	cgm_vcons(&pvel, 0, 0, 0);

	cam[0].dist = 10;
	cam[0].height = 3;
	cam[0].roll = 0;
	cam_follow(cam, &ppos, &pdir);

	/* loading done */
	draw = race_draw;
	key_event = race_keyb;

	/* save menu video mode, and switch to game video mode */
	menu_mode_idx = get_video_mode(VMODE_CURRENT) - video_modes();
	if((vmidx = match_video_mode(opt.xres, opt.yres, opt.bpp)) != -1) {
		vmem = set_video_mode(vmidx, 1);
	}

	g3d_reset();
	g3d_framebuffer(fb_width, fb_height, fb_pixels);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(50.0, (float)fb_width / (float)fb_height, 0.5, 1000.0);

	g3d_enable(G3D_CULL_FACE);
	/*g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);*/

	prev_upd = time_msec;
}

void race_stop(void)
{
	destroy_track(&trk);
	free_curve(path);

	if(menu_mode_idx >= 0) {
		vmem = set_video_mode(menu_mode_idx, 1);
		menu_mode_idx = -1;
	}
}

#define DRAG	1.0
#define BRK		0.1

static void update(void)
{
	cgm_vec3 targ, up = {0, 1, 0};
	float dt, s, brk;
	long dt_ms = time_msec - prev_upd;
	prev_upd = time_msec;

	dt = dt_ms / 1000.0f;

	brk = DRAG;

	if(inpstate[INP_FWD]) {
		cgm_vadd_scaled(&pvel, &pdir, dt * 50.0);
	}
	if(inpstate[INP_BRK]) {
		brk += BRK;
	}
	if(inpstate[INP_LTURN]) {
		cgm_vrotate(&pdir, dt * 0.5, 0, 1, 0);	/* TODO take roll into account */
	}
	if(inpstate[INP_RTURN]) {
		cgm_vrotate(&pdir, -dt * 0.5, 0, 1, 0);
	}

	pspeed = cgm_vlength(&pvel) - brk;
	s = pspeed == 0.0f ? 0.0f : 1.0f / pspeed;

	if(pspeed < 0) pspeed = 0;
	s *= pspeed;
	cgm_vscale(&pvel, s * dt);

	cgm_vadd_scaled(&ppos, &pvel, dt);

	projt = curve_proj_guess(path, &ppos, projt, 0.001, &proj_pos);
	ppos.y = proj_pos.y;

	targ = ppos;
	cgm_vadd(&targ, &pdir);
	cgm_mlookat(pxform, &ppos, &targ, &up);

	cam_follow(cam, &ppos, &pdir);
}

void race_draw(void)
{
	int i;

	update();
	memset(fb_pixels, 0, fb_size);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_matrix(cam[act_cam].matrix);

	g3d_set_texture(road_tex_width, road_tex_height, road_tex);
	g3d_enable(G3D_TEXTURE_2D);
	for(i=0; i<trk.num_tseg; i++) {
		draw_mesh(&trk.tseg[i].mesh);
	}

	g3d_push_matrix();
	g3d_mult_matrix(pxform);

	g3d_set_texture(ship_tex_width, ship_tex_height, ship_tex);
	zsort_mesh(&ship_mesh);
	draw_mesh(&ship_mesh);

	g3d_pop_matrix();

	g3d_disable(G3D_TEXTURE_2D);
	g3d_begin(G3D_LINES);
	g3d_color3b(255, 92, 92);
	g3d_vertex(proj_pos.x, proj_pos.y, proj_pos.z);
	g3d_vertex(proj_pos.x + 0.5, proj_pos.y, proj_pos.z);
	g3d_color3b(92, 255, 92);
	g3d_vertex(proj_pos.x, proj_pos.y, proj_pos.z);
	g3d_vertex(proj_pos.x, proj_pos.y + 0.5, proj_pos.z);
	g3d_color3b(92, 92, 255);
	g3d_vertex(proj_pos.x, proj_pos.y, proj_pos.z);
	g3d_vertex(proj_pos.x, proj_pos.y, proj_pos.z + 0.5);
	g3d_end();

	select_font(FONT_VGA);
	fnt_align(FONT_LEFT);
	fnt_printf(fb_pixels, 0, 20, "t:%.3f", projt);

	blit_frame(fb_pixels, 0);
}

void race_keyb(int key, int pressed)
{
	int i;

	for(i=0; i<NUM_INPUTS; i++) {
		if(key == keymap[i][0] || key == keymap[i][1]) {
			inpstate[i] = pressed;
		}
	}

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
