#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "miniglut.h"
#include "game.h"
#include "gfx.h"
#include "gfxutil.h"
#include "timer.h"
#include "joy.h"
#include "input.h"
#include "audio.h"
#include "options.h"

#undef GLDEBUG

static void display(void);
static void idle(void);
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void skeydown(int key, int x, int y);
static void skeyup(int key, int x, int y);
static int translate_special(int skey);
static unsigned int next_pow2(unsigned int x);
static void set_fullscreen(int fs);
static void set_vsync(int vsync);
#ifdef GLDEBUG
static void gldebug(unsigned int src, unsigned int type, unsigned int id,
		unsigned int severity, int length, const char *msg, const void *cls);
#endif

int have_joy;
unsigned int joy_bnstate, joy_bndiff, joy_bnpress;

#define MODE(w, h)	\
	{0, w, h, 16, w * 2, 5, 6, 5, 11, 5, 0, 0xf800, 0x7e0, 0x1f, 0xbadf00d, 2, 0}
static struct video_mode vmodes[] = {
	MODE(320, 240), MODE(400, 300), MODE(512, 384), MODE(640, 480),
	MODE(800, 600), MODE(1024, 768), MODE(1280, 960), MODE(1280, 1024),
	MODE(1920, 1080), MODE(1600, 1200), MODE(1920, 1200)
};
static struct video_mode *cur_vmode;

static unsigned int num_pressed;
static unsigned char keystate[256];

static unsigned long start_time;
static unsigned int modkeys;

static float win_aspect;
static unsigned int tex;

#ifdef __unix__
#include <GL/glx.h>
static Display *xdpy;
static Window xwin;

static void (*glx_swap_interval_ext)();
static void (*glx_swap_interval_sgi)();
#endif
#ifdef _WIN32
#include <windows.h>
static PROC wgl_swap_interval_ext;
#endif

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("eradicate - alt+enter for fullscreen");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(skeydown);
	glutSpecialUpFunc(skeyup);

	glutSetCursor(GLUT_CURSOR_NONE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);

#ifdef GLDEBUG
	glDebugMessageCallback(gldebug, 0);
	glEnable(GL_DEBUG_OUTPUT);
#endif

	if(!set_video_mode(match_video_mode(640, 480, 16), 1)) {
		return 1;
	}

#ifdef __unix__
	xdpy = glXGetCurrentDisplay();
	xwin = glXGetCurrentDrawable();

	if(!(glx_swap_interval_ext = glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT"))) {
		glx_swap_interval_sgi = glXGetProcAddress((unsigned char*)"glXSwapIntervalSGI");
	}
#endif
#ifdef _WIN32
	wgl_swap_interval_ext = wglGetProcAddress("wglSwapIntervalEXT");
#endif

	if(au_init() == -1) {
		return 1;
	}
	time_msec = 0;
	if(init(argc, argv) == -1) {
		return 1;
	}
	atexit(cleanup);

	if(opt.fullscreen) {
		set_fullscreen(opt.fullscreen);
	}

	reset_timer();

	glutMainLoop();
	return 0;
}

void game_quit(void)
{
	exit(0);
}

struct video_mode *video_modes(void)
{
	return vmodes;
}

int num_video_modes(void)
{
	return sizeof vmodes / sizeof *vmodes;
}

struct video_mode *get_video_mode(int idx)
{
	if(idx == VMODE_CURRENT) {
		return cur_vmode;
	}
	return vmodes + idx;
}

int match_video_mode(int xsz, int ysz, int bpp)
{
	struct video_mode *vm = vmodes;
	int i, count = num_video_modes();

	for(i=0; i<count; i++) {
		if(vm->xsz == xsz && vm->ysz == ysz && vm->bpp == bpp) {
			return i;
		}
		vm++;
	}
	return -1;
}

static int tex_xsz, tex_ysz;
static uint32_t *convbuf;
static int convbuf_size;

void *set_video_mode(int idx, int nbuf)
{
	struct video_mode *vm = vmodes + idx;

	if(cur_vmode == vm) {
		return vmem;
	}

#ifndef BUILD_OPENGL
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	tex_xsz = next_pow2(vm->xsz);
	tex_ysz = next_pow2(vm->ysz);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_xsz, tex_ysz, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, 0);
	if(opt.scaler == SCALER_LINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef((float)vm->xsz / tex_xsz, (float)vm->ysz / tex_ysz, 1);

	if(vm->xsz * vm->ysz > convbuf_size) {
		convbuf_size = vm->xsz * vm->ysz;
		free(convbuf);
		convbuf = malloc(convbuf_size * sizeof *convbuf);
	}
#endif	/* !def BUILD_OPENGL */

	if(resizefb(vm->xsz, vm->ysz, vm->bpp) == -1) {
		fprintf(stderr, "failed to allocate virtual framebuffer\n");
		return 0;
	}
	vmem = fb_pixels;

	cur_vmode = vm;
	return vmem;
}

int set_text_mode(void)
{
	return 0;
}

void wait_vsync(void)
{
}

void blit_frame(void *pixels, int vsync)
{
	int i, npix = fb_width * fb_height;
	uint32_t *dptr = convbuf;
	uint16_t *sptr = pixels;
	static int prev_vsync = -1;

	if(vsync != prev_vsync) {
		set_vsync(vsync);
		prev_vsync = vsync;
	}

	if(show_fps) dbg_fps(pixels);

#ifndef BUILD_OPENGL
	for(i=0; i<npix; i++) {
		int r = UNPACK_R16(*sptr);
		int g = UNPACK_G16(*sptr);
		int b = UNPACK_B16(*sptr);
		*dptr++ = PACK_RGB32(b, g, r);
		sptr++;
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fb_width, fb_height, GL_RGBA,
			GL_UNSIGNED_BYTE, convbuf);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(win_aspect >= fb_aspect) {
		glScalef(fb_aspect / win_aspect, 1, 1);
	} else {
		glScalef(1, win_aspect / fb_aspect, 1);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-1, 1);
	glEnd();
#endif	/* !def BUILD_OPENGL */

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

/* joystick */
int joy_detect(void)
{
	return 0;
}

void joy_update(void)
{
}

int kb_isdown(int key)
{
	switch(key) {
	case KB_ANY:
		return num_pressed;

	case KB_ALT:
		return keystate[KB_LALT] + keystate[KB_RALT];

	case KB_CTRL:
		return keystate[KB_LCTRL] + keystate[KB_RCTRL];
	}

	if(isalpha(key)) {
		key = tolower(key);
	}
	return keystate[key];
}

/* timer */
void init_timer(int res_hz)
{
}

void reset_timer(void)
{
	start_time = glutGet(GLUT_ELAPSED_TIME);
}

unsigned long get_msec(void)
{
	return glutGet(GLUT_ELAPSED_TIME) - start_time;
}

static void display(void)
{
	inp_update();

#ifdef BUILD_OPENGL
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	time_msec = get_msec();
	draw();
}

static void idle(void)
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	win_aspect = (float)x / (float)y;
#ifdef BUILD_OPENGL
	if(!fb_aspect) fb_aspect = 1.3333333333f;
	if(fb_aspect <= win_aspect) {
		float gap = x - y * fb_aspect;
		glViewport(gap / 2, 0, y * fb_aspect, y);
	} else {
		float gap = y - x / fb_aspect;
		glViewport(0, gap / 2, x, x / fb_aspect);
	}
#else
	glViewport(0, 0, x, y);
#endif
}

static void keydown(unsigned char key, int x, int y)
{
	modkeys = glutGetModifiers();

	if((key == '\n' || key == '\r') && (modkeys & GLUT_ACTIVE_ALT)) {
		opt.fullscreen ^= 1;
		set_fullscreen(opt.fullscreen);
		return;
	}
	keystate[key] = 1;
	game_key(key, 1);
}

static void keyup(unsigned char key, int x, int y)
{
	keystate[key] = 0;
	game_key(key, 0);
}

static void skeydown(int key, int x, int y)
{
	if(key == GLUT_KEY_F5) {
#ifndef BUILD_OPENGL
		opt.scaler = (opt.scaler + 1) % NUM_SCALERS;

		if(opt.scaler == SCALER_LINEAR) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
#endif	/* !def BUILD_OPENGL */
	}
	key = translate_special(key);
	keystate[key] = 1;
	game_key(key, 1);
}

static void skeyup(int key, int x, int y)
{
	key = translate_special(key);
	keystate[key] = 0;
	game_key(key, 0);
}

static int translate_special(int skey)
{
	switch(skey) {
	case 127:
		return 127;
	case GLUT_KEY_LEFT:
		return KB_LEFT;
	case GLUT_KEY_RIGHT:
		return KB_RIGHT;
	case GLUT_KEY_UP:
		return KB_UP;
	case GLUT_KEY_DOWN:
		return KB_DOWN;
	case GLUT_KEY_PAGE_UP:
		return KB_PGUP;
	case GLUT_KEY_PAGE_DOWN:
		return KB_PGDN;
	case GLUT_KEY_HOME:
		return KB_HOME;
	case GLUT_KEY_END:
		return KB_END;
	default:
		if(skey >= GLUT_KEY_F1 && skey <= GLUT_KEY_F12) {
			return KB_F1 + skey - GLUT_KEY_F1;
		}
	}
	return 0;
}

static unsigned int next_pow2(unsigned int x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

static void set_fullscreen(int fs)
{
	static int win_x, win_y;

	if(fs) {
		win_x = glutGet(GLUT_WINDOW_WIDTH);
		win_y = glutGet(GLUT_WINDOW_HEIGHT);
		glutFullScreen();
	} else {
		glutReshapeWindow(win_x, win_y);
	}
}

#ifdef __unix__
static void set_vsync(int vsync)
{
	vsync = vsync ? 1 : 0;
	if(glx_swap_interval_ext) {
		glx_swap_interval_ext(xdpy, xwin, vsync);
	} else if(glx_swap_interval_sgi) {
		glx_swap_interval_sgi(vsync);
	}
}
#endif
#ifdef WIN32
static void set_vsync(int vsync)
{
	if(wgl_swap_interval_ext) {
		wgl_swap_interval_ext(vsync ? 1 : 0);
	}
}
#endif

#ifdef GLDEBUG
static void gldebug(unsigned int src, unsigned int type, unsigned int id,
		unsigned int severity, int length, const char *msg, const void *cls)
{
	fprintf(stderr, "GLDEBUG: %s\n", msg);
}
#endif
