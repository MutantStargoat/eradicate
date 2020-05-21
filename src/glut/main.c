#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <GL/glut.h>
#include "game.h"
#include "gfx.h"
#include "timer.h"
#include "joy.h"
#include "input.h"
#include "audio.h"


static void display(void);
static void idle(void);
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void skeydown(int key, int x, int y);
static void skeyup(int key, int x, int y);
static unsigned int next_pow2(unsigned int x);
static void set_vsync(int vsync);

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

#ifdef __unix__
#include <GL/glx.h>
static Display *xdpy;
static Window xwin;

static void (*glx_swap_interval_ext)();
static void (*glx_swap_interval_sgi)();
#endif
#ifdef WIN32
#include <windows.h>
static int (*wgl_swap_interval_ext)(int);
#endif

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutCreateWindow("eradicate/GLUT");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(skeydown);
	glutSpecialUpFunc(skeyup);

	glutSetCursor(GLUT_CURSOR_NONE);

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
#ifdef WIN32
	wgl_swap_interval_ext = wglGetProcAddress("wglSwapIntervalEXT");
#endif
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);

	if(au_init() == -1) {
		return 1;
	}
	time_msec = 0;
	if(init(argc, argv) == -1) {
		return 1;
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

void *set_video_mode(int idx, int nbuf)
{
	struct video_mode *vm = vmodes + idx;

	if(cur_vmode == vm) {
		return vmem;
	}

	tex_xsz = next_pow2(vm->xsz);
	tex_ysz = next_pow2(vm->ysz);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_xsz, tex_ysz, 0, GL_RGB,
			GL_UNSIGNED_SHORT_5_6_5, 0);

	if(resizefb(vm->xsz, vm->ysz, vm->bpp) == -1) {
		fprintf(stderr, "failed to allocate virtual framebuffer\n");
		return 0;
	}
	vmem = fb_pixels;

	cur_vmode = vm;
	return vmem;
}

void wait_vsync(void)
{
}

void blit_frame(void *pixels, int vsync)
{
	static int prev_vsync = -1;

	if(vsync != prev_vsync) {
		set_vsync(vsync);
		prev_vsync = vsync;
	}

	if(show_fps) dbg_fps(pixels);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fb_width, fb_height, GL_RGB,
			GL_UNSIGNED_SHORT_5_6_5, pixels);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(-1, 1);
	glTexCoord2f(1, 0);
	glVertex2f(1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(1, -1);
	glEnd();

	glutSwapBuffers();
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

	time_msec = get_msec();
	draw();
}

static void idle(void)
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
}

static void keydown(unsigned char key, int x, int y)
{
}

static void keyup(unsigned char key, int x, int y)
{
}

static void skeydown(int key, int x, int y)
{
}

static void skeyup(int key, int x, int y)
{
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

#ifdef __unix__
static void set_vsync(int vsync)
{
	if(glx_swap_interval_ext) {
		glx_swap_interval_ext(xdpy, xwin, vsync ? 1 : 0);
	} else if(glx_swap_interval_sgi) {
		glx_swap_interval_sgi(vsync ? 1 : 0);
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
