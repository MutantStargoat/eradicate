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
#include "util.h"

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

#define CAM_HEIGHT	2.0f
#define CAM_DIST	10.0f

#define TRK_SUBDIV	26
#define TRK_TWIST	30
#define COL_DIST	5.0

#define ACCEL		10.0f
#define DRAG		1.0f
#define BRK			12.0f
#define MAX_SPEED	100.0f
#define MAX_TURN_RATE	1.5f
#define COL_BRK		1.2f

#define MAX_ROLL	32.0f

static void draw_ui(void);

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
static float pspeed, proll;
static float turn_rate;
static float pxform[16];
static float projt;
static cgm_vec3 proj_pos;

static long prev_upd;

static int cur_seg;
static int nseg_to_draw = 2;
static int wrong_way;

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
	cur_seg = 0;

	cam[0].dist = CAM_DIST;
	cam[0].roll = 0;
	cam_follow(cam, &ppos, &pdir, CAM_HEIGHT);

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

#define CLAMP(x, a, b)	((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

#define TURN_MORE(d)	\
	do { \
		turn_rate += pspeed < 0.001f ? 0.0f : (d) * dt / pspeed; \
		turn_rate = CLAMP(turn_rate, -MAX_TURN_RATE, MAX_TURN_RATE); \
		proll += (d) * 0.5f * dt; \
		proll = CLAMP(proll, -MAX_ROLL, MAX_ROLL); \
	} while(0)

static void update(void)
{
	cgm_vec3 targ, up = {0, 1, 0};
	cgm_vec3 offs_dir, path_dir;
	float dt, s, lensq;
	long dt_ms = time_msec - prev_upd;
	prev_upd = time_msec;

	dt = dt_ms / 1000.0f;

	pspeed -= DRAG * dt;

	if(inpstate[INP_FWD]) {
		pspeed += ACCEL * dt;
	}
	if(inpstate[INP_BRK]) {
		pspeed -= BRK * dt;
	}

	if(pspeed < 0) pspeed = 0;
	if(pspeed > MAX_SPEED) pspeed = MAX_SPEED;
	cgm_vadd_scaled(&ppos, &pdir, pspeed * dt);

	if(inpstate[INP_LTURN]) {
		TURN_MORE(128.0f);
		cgm_vrotate(&pdir, dt * turn_rate, 0, 1, 0);	/* TODO take roll into account */
	} else if(inpstate[INP_RTURN]) {
		TURN_MORE(-128.0f);
		cgm_vrotate(&pdir, dt * turn_rate, 0, 1, 0);
	} else {
		proll *= 0.1f;
		if(fabs(proll) < 0.01f) proll = 0.0f;
		turn_rate = 0;
	}

	projt = curve_proj_guess(path, &ppos, projt, 0.001, &proj_pos);
	cur_seg = projt * trk.num_tseg;
	if(cur_seg >= trk.num_tseg) cur_seg -= trk.num_tseg;
	ppos.y = proj_pos.y;

	eval_curve(path, projt + 0.001, &path_dir);
	cgm_vsub(&path_dir, &proj_pos);
	wrong_way = cgm_vdot(&path_dir, &pdir) < 0.0f;

	/* collision */
	offs_dir = ppos;
	cgm_vsub(&offs_dir, &proj_pos);
	if((lensq = cgm_vlength_sq(&offs_dir)) > COL_DIST * COL_DIST) {
		s = rsqrt(lensq) * COL_DIST;
		ppos.x = proj_pos.x + offs_dir.x * s;
		ppos.y = proj_pos.y + offs_dir.y * s;
		ppos.z = proj_pos.z + offs_dir.z * s;
		pspeed -= pspeed * COL_BRK * dt;
		if(pspeed < 0.0f) pspeed = 0.0f;
	}

	targ = ppos;
	cgm_vadd(&targ, &pdir);
	cgm_mlookat(pxform, &ppos, &targ, &up);

	cgm_vadd_scaled(&targ, &up, 1.0f);
	cam_follow_step(cam, &targ, &pdir, CAM_HEIGHT, 5.0f * dt);
}


void race_draw(void)
{
	int i, seg, inc;

	update();
	memset(fb_pixels, 0, fb_size);

	g3d_polygon_mode(G3D_FLAT);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_matrix(cam[act_cam].matrix);

	g3d_set_texture(road_tex_width, road_tex_height, road_tex);
	g3d_enable(G3D_TEXTURE_2D);

	/* draw nseg_to_draw segments in front in back to front order
	 * and one segment back (nseg_to_draw + 1 and keep decrementing)
	 */
	inc = wrong_way ? -1 : 1;
	seg = cur_seg + inc * (nseg_to_draw - 1);
	if(seg < 0) {
		seg += trk.num_tseg;
	} else if(seg >= trk.num_tseg) {
		seg -= trk.num_tseg;
	}
	for(i=0; i<nseg_to_draw + 1; i++) {
		draw_mesh(&trk.tseg[seg].mesh);
		seg -= inc;
		if(seg < 0) {
			seg = trk.num_tseg - 1;
		} else if(seg >= trk.num_tseg) {
			seg = 0;
		}
	}

	g3d_push_matrix();
	g3d_mult_matrix(pxform);
	g3d_rotate(proll, 0, 0, 1);

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

	draw_ui();

	blit_frame(fb_pixels, 0);
}

static const struct g3d_vertex speedo_verts[] = {
	{0.139, 0.000, 0,	1, 0, 0, 0, 0, 0,	13, 140, 156, 255},
	{0.197, 0.106, 0,	1, 0, 0, 0, 0, 0,	13, 139, 146, 255},
	{0.267, 0.227, 0,	1, 0, 0, 0, 0, 0,	41, 137, 96, 255},
	{0.348, 0.318, 0,	1, 0, 0, 0, 0, 0,	71, 125, 52, 255},
	{0.453, 0.410, 0,	1, 0, 0, 0, 0, 0,	111, 105, 0, 255},
	{0.569, 0.484, 0,	1, 0, 0, 0, 0, 0,	153, 84, 0, 255},
	{0.697, 0.545, 0,	1, 0, 0, 0, 0, 0,	182, 69, 0, 255},
	{0.889, 0.606, 0,	1, 0, 0, 0, 0, 0,	206, 57, 0, 255},
	{1.104, 0.615, 0,	1, 0, 0, 0, 0, 0,	253, 33, 0, 255},
	{0.011, 0.060, 0,	1, 0, 0, 0, 0, 0,	13, 140, 156, 255},
	{0.065, 0.196, 0,	1, 0, 0, 0, 0, 0,	13, 139, 146, 255},
	{0.139, 0.363, 0,	1, 0, 0, 0, 0, 0,	41, 137, 96, 255},
	{0.209, 0.500, 0,	1, 0, 0, 0, 0, 0,	71, 125, 52, 255},
	{0.302, 0.651, 0,	1, 0, 0, 0, 0, 0,	111, 105, 0, 255},
	{0.441, 0.803, 0,	1, 0, 0, 0, 0, 0,	153, 84, 0, 255},
	{0.616, 0.920, 0,	1, 0, 0, 0, 0, 0,	182, 69, 0, 255},
	{0.883, 1.000, 0,	1, 0, 0, 0, 0, 0,	206, 57, 0, 255},
	{1.200, 1.010, 0,	1, 0, 0, 0, 0, 0,	253, 33, 0, 255},
};
static const uint16_t speedo_idx[] = {
	0, 1, 10, 9, 1, 2, 11, 10, 2, 3, 12, 11, 3, 4, 13, 12,
	4, 5, 14, 13, 5, 6, 15, 14, 6, 7, 16, 15, 7, 8, 17, 16
};
#define SPEEDO_NVERTS	(sizeof speedo_verts / sizeof *speedo_verts)
#define SPEEDO_NIDX		(sizeof speedo_idx / sizeof *speedo_idx)

static void draw_ui(void)
{
	int speed_level;

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_push_matrix();
	g3d_load_identity();
	g3d_ortho(0, fb_width, 0, fb_height, -1, 1);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();

	/* draw speedometer */
	g3d_translate(fb_width * 0.7, fb_height * 0.05, 0);
	g3d_scale(fb_width / 5.0f, fb_height / 6.0f, 1);
	g3d_polygon_mode(G3D_FLAT);
	speed_level = pspeed * 8.0f / MAX_SPEED + 0.5f;
	g3d_draw_indexed(G3D_QUADS, speedo_verts, SPEEDO_NVERTS, speedo_idx, speed_level * 4);
	g3d_polygon_mode(G3D_WIRE);
	g3d_enable(G3D_LIGHTING);
	g3d_draw_indexed(G3D_QUADS, speedo_verts, SPEEDO_NVERTS, speedo_idx, SPEEDO_NIDX);
	g3d_disable(G3D_LIGHTING);

	if(wrong_way && (time_msec & 0x500)) {
		select_font(fb_width > 400 ? FONT_MENU_SHADED_BIG : FONT_MENU_SHADED);
		fnt_align(FONT_CENTER);
		fnt_print(fb_pixels, fb_width / 2, fb_height / 8, "WRONG WAY!");
		/* TODO do something resolution-independent (or just provide a few sizes of fonts) */
	}

	select_font(FONT_VGA);
	fnt_align(FONT_LEFT);
	fnt_printf(fb_pixels, 0, 20, "t:%.3f", projt);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_pop_matrix();
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
