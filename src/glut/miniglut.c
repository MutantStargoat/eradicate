/*
MiniGLUT - minimal GLUT subset without dependencies
Copyright (C) 2020  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifdef MINIGLUT_USE_LIBC
#define _GNU_SOURCE
#include <stdlib.h>
#include <math.h>
#else
static void mglut_sincosf(float angle, float *sptr, float *cptr);
static float mglut_atan(float x);
#endif

#define PI	3.1415926536f

#if defined(__unix__)

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#define BUILD_X11

#ifndef GLX_SAMPLE_BUFFERS_ARB
#define GLX_SAMPLE_BUFFERS_ARB	100000
#define GLX_SAMPLES_ARB			100001
#endif
#ifndef GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB
#define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB	0x20b2
#endif

static Display *dpy;
static Window win, root;
static int scr;
static GLXContext ctx;
static Atom xa_wm_proto, xa_wm_del_win;
static unsigned int evmask;

#elif defined(_WIN32)

#include <windows.h>
#define BUILD_WIN32

static HRESULT CALLBACK handle_message(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);

static HINSTANCE hinst;
static HWND win;
static HDC dc;
static HGLRC ctx;

#else
#error unsupported platform
#endif

#include <GL/gl.h>
#include "miniglut.h"

struct ctx_info {
	int rsize, gsize, bsize, asize;
	int zsize, ssize;
	int dblbuf;
	int samples;
	int stereo;
	int srgb;
};

static void create_window(const char *title);
static void get_window_pos(int *x, int *y);
static void get_window_size(int *w, int *h);
static void get_screen_size(int *scrw, int *scrh);

static long get_msec(void);
static void panic(const char *msg);
static void sys_exit(int status);
static int sys_write(int fd, const void *buf, int count);


static int init_x, init_y, init_width = 256, init_height = 256;
static unsigned int init_mode;

static struct ctx_info ctx_info;
static int cur_cursor = GLUT_CURSOR_INHERIT;

static glut_cb cb_display;
static glut_cb cb_idle;
static glut_cb_reshape cb_reshape;
static glut_cb_state cb_vis, cb_entry;
static glut_cb_keyb cb_keydown, cb_keyup;
static glut_cb_special cb_skeydown, cb_skeyup;
static glut_cb_mouse cb_mouse;
static glut_cb_motion cb_motion, cb_passive;
static glut_cb_sbmotion cb_sball_motion, cb_sball_rotate;
static glut_cb_sbbutton cb_sball_button;

static int win_width, win_height;
static int mapped;
static int quit;
static int upd_pending;
static int modstate;


void glutInit(int *argc, char **argv)
{
#ifdef BUILD_X11
	if(!(dpy = XOpenDisplay(0))) {
		panic("Failed to connect to the X server\n");
	}
	scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);
	xa_wm_proto = XInternAtom(dpy, "WM_PROTOCOLS", False);
	xa_wm_del_win = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	evmask = ExposureMask | StructureNotifyMask;
#endif
#ifdef BUILD_WIN32
	WNDCLASSEX wc = {0};

	hinst = GetModuleHandle(0);

	wc.cbSize = sizeof wc;
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hinst;
	wc.lpfnWndProc = handle_message;
	wc.lpszClassName = "MiniGLUT";
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	if(!RegisterClassEx(&wc)) {
		panic("Failed to register \"MiniGLUT\" window class\n");
	}
#endif
}

void glutInitWindowPosition(int x, int y)
{
	init_x = x;
	init_y = y;
}

void glutInitWindowSize(int xsz, int ysz)
{
	init_width = xsz;
	init_height = ysz;
}

void glutInitDisplayMode(unsigned int mode)
{
	init_mode = mode;
}

void glutCreateWindow(const char *title)
{
	create_window(title);
}

void glutExit(void)
{
	quit = 1;
}

void glutMainLoop(void)
{
	while(!quit) {
		glutMainLoopEvent();
	}
}

void glutPostRedisplay(void)
{
	upd_pending = 1;
}

#define UPD_EVMASK(x) \
	do { \
		if(func) { \
			evmask |= x; \
		} else { \
			evmask &= ~(x); \
		} \
		if(win) XSelectInput(dpy, win, evmask); \
	} while(0)


void glutIdleFunc(glut_cb func)
{
	cb_idle = func;
}

void glutDisplayFunc(glut_cb func)
{
	cb_display = func;
}

void glutReshapeFunc(glut_cb_reshape func)
{
	cb_reshape = func;
}

void glutVisibilityFunc(glut_cb_state func)
{
	cb_vis = func;
#ifdef BUILD_X11
	UPD_EVMASK(VisibilityChangeMask);
#endif
}

void glutEntryFunc(glut_cb_state func)
{
	cb_entry = func;
#ifdef BUILD_X11
	UPD_EVMASK(EnterWindowMask | LeaveWindowMask);
#endif
}

void glutKeyboardFunc(glut_cb_keyb func)
{
	cb_keydown = func;
#ifdef BUILD_X11
	UPD_EVMASK(KeyPressMask);
#endif
}

void glutKeyboardUpFunc(glut_cb_keyb func)
{
	cb_keyup = func;
#ifdef BUILD_X11
	UPD_EVMASK(KeyReleaseMask);
#endif
}

void glutSpecialFunc(glut_cb_special func)
{
	cb_skeydown = func;
#ifdef BUILD_X11
	UPD_EVMASK(KeyPressMask);
#endif
}

void glutSpecialUpFunc(glut_cb_special func)
{
	cb_skeyup = func;
#ifdef BUILD_X11
	UPD_EVMASK(KeyReleaseMask);
#endif
}

void glutMouseFunc(glut_cb_mouse func)
{
	cb_mouse = func;
#ifdef BUILD_X11
	UPD_EVMASK(ButtonPressMask | ButtonReleaseMask);
#endif
}

void glutMotionFunc(glut_cb_motion func)
{
	cb_motion = func;
#ifdef BUILD_X11
	UPD_EVMASK(ButtonMotionMask);
#endif
}

void glutPassiveMotionFunc(glut_cb_motion func)
{
	cb_passive = func;
#ifdef BUILD_X11
	UPD_EVMASK(PointerMotionMask);
#endif
}

void glutSpaceballMotionFunc(glut_cb_sbmotion func)
{
	cb_sball_motion = func;
}

void glutSpaceballRotateFunc(glut_cb_sbmotion func)
{
	cb_sball_rotate = func;
}

void glutSpaceballBittonFunc(glut_cb_sbbutton func)
{
	cb_sball_button = func;
}

int glutGet(unsigned int s)
{
	int x, y;
	switch(s) {
	case GLUT_WINDOW_X:
		get_window_pos(&x, &y);
		return x;
	case GLUT_WINDOW_Y:
		get_window_pos(&x, &y);
		return y;
	case GLUT_WINDOW_WIDTH:
		get_window_size(&x, &y);
		return x;
	case GLUT_WINDOW_HEIGHT:
		get_window_size(&x, &y);
		return y;
	case GLUT_WINDOW_BUFFER_SIZE:
		return ctx_info.rsize + ctx_info.gsize + ctx_info.bsize + ctx_info.asize;
	case GLUT_WINDOW_STENCIL_SIZE:
		return ctx_info.ssize;
	case GLUT_WINDOW_DEPTH_SIZE:
		return ctx_info.zsize;
	case GLUT_WINDOW_RED_SIZE:
		return ctx_info.rsize;
	case GLUT_WINDOW_GREEN_SIZE:
		return ctx_info.gsize;
	case GLUT_WINDOW_BLUE_SIZE:
		return ctx_info.bsize;
	case GLUT_WINDOW_ALPHA_SIZE:
		return ctx_info.asize;
	case GLUT_WINDOW_DOUBLEBUFFER:
		return ctx_info.dblbuf;
	case GLUT_WINDOW_RGBA:
		return 1;
	case GLUT_WINDOW_NUM_SAMPLES:
		return ctx_info.samples;
	case GLUT_WINDOW_STEREO:
		return ctx_info.stereo;
	case GLUT_WINDOW_SRGB:
		return ctx_info.srgb;
	case GLUT_WINDOW_CURSOR:
		return cur_cursor;
	case GLUT_SCREEN_WIDTH:
		get_screen_size(&x, &y);
		return x;
	case GLUT_SCREEN_HEIGHT:
		get_screen_size(&x, &y);
		return y;
	case GLUT_INIT_DISPLAY_MODE:
		return init_mode;
	case GLUT_INIT_WINDOW_X:
		return init_x;
	case GLUT_INIT_WINDOW_Y:
		return init_y;
	case GLUT_INIT_WINDOW_WIDTH:
		return init_width;
	case GLUT_INIT_WINDOW_HEIGHT:
		return init_height;
	case GLUT_ELAPSED_TIME:
		return get_msec();
	default:
		break;
	}
	return 0;
}

int glutGetModifiers(void)
{
	return modstate;
}

static int is_space(int c)
{
	return c == ' ' || c == '\t' || c == '\v' || c == '\n' || c == '\r';
}

static const char *skip_space(const char *s)
{
	while(*s && is_space(*s)) s++;
	return s;
}

int glutExtensionSupported(char *ext)
{
	const char *str, *eptr;

	if(!(str = (const char*)glGetString(GL_EXTENSIONS))) {
		return 0;
	}

	while(*str) {
		str = skip_space(str);
		eptr = skip_space(ext);
		while(*str && !is_space(*str) && *eptr && *str == *eptr) {
			str++;
			eptr++;
		}
		if((!*str || is_space(*str)) && !*eptr) {
			return 1;
		}
		while(*str && !is_space(*str)) str++;
	}

	return 0;
}

/* TODO */
void glutSolidSphere(float rad, int slices, int stacks)
{
	int i, j, k, gray;
	float x, y, z, s, t, u, v, phi, theta, sintheta, costheta, sinphi, cosphi;
	float du = 1.0f / (float)slices;
	float dv = 1.0f / (float)stacks;

	glBegin(GL_QUADS);
	for(i=0; i<stacks; i++) {
		v = i * dv;
		for(j=0; j<slices; j++) {
			u = j * du;
			for(k=0; k<4; k++) {
				gray = k ^ (k >> 1);
				s = gray & 1 ? u + du : u;
				t = gray & 2 ? v + dv : v;
				theta = s * PI * 2.0f;
				phi = t * PI;
				mglut_sincosf(theta, &sintheta, &costheta);
				mglut_sincosf(phi, &sinphi, &cosphi);
				x = sintheta * sinphi;
				y = costheta * sinphi;
				z = cosphi;

				glColor3f(s, t, 1);
				glTexCoord2f(s, t);
				glNormal3f(x, y, z);
				glVertex3f(x * rad, y * rad, z * rad);
			}
		}
	}
	glEnd();
}

void glutWireSphere(float rad, int slices, int stacks)
{
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSolidSphere(rad, slices, stacks);
	glPopAttrib();
}

void glutSolidCube(float sz)
{
	int i, j, idx, gray, flip, rotx;
	float vpos[3], norm[3];
	float rad = sz * 0.5f;

	glBegin(GL_QUADS);
	for(i=0; i<6; i++) {
		flip = i & 1;
		rotx = i >> 2;
		idx = (~i & 2) - rotx;
		norm[0] = norm[1] = norm[2] = 0.0f;
		norm[idx] = flip ^ ((i >> 1) & 1) ? -1 : 1;
		glNormal3fv(norm);
		vpos[idx] = norm[idx] * rad;
		for(j=0; j<4; j++) {
			gray = j ^ (j >> 1);
			vpos[i & 2] = (gray ^ flip) & 1 ? rad : -rad;
			vpos[rotx + 1] = (gray ^ (rotx << 1)) & 2 ? rad : -rad;
			glTexCoord2f(gray & 1, gray >> 1);
			glVertex3fv(vpos);
		}
	}
	glEnd();
}

void glutWireCube(float sz)
{
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSolidCube(sz);
	glPopAttrib();
}

static void draw_cylinder(float rbot, float rtop, float height, int slices, int stacks)
{
	int i, j, k, gray;
	float x, y, z, s, t, u, v, theta, phi, sintheta, costheta, sinphi, cosphi, rad;
	float du = 1.0f / (float)slices;
	float dv = 1.0f / (float)stacks;

	rad = rbot - rtop;
	phi = mglut_atan((rad < 0 ? -rad : rad) / height);
	mglut_sincosf(phi, &sinphi, &cosphi);

	glBegin(GL_QUADS);
	for(i=0; i<stacks; i++) {
		v = i * dv;
		for(j=0; j<slices; j++) {
			u = j * du;
			for(k=0; k<4; k++) {
				gray = k ^ (k >> 1);
				s = gray & 2 ? u + du : u;
				t = gray & 1 ? v + dv : v;
				rad = rbot + (rtop - rbot) * t;
				theta = s * PI * 2.0f;
				mglut_sincosf(theta, &sintheta, &costheta);

				x = sintheta * cosphi;
				y = costheta * cosphi;
				z = sinphi;

				glColor3f(s, t, 1);
				glTexCoord2f(s, t);
				glNormal3f(x, y, z);
				glVertex3f(sintheta * rad, costheta * rad, t * height);
			}
		}
	}
	glEnd();
}

void glutSolidCone(float base, float height, int slices, int stacks)
{
	draw_cylinder(base, 0, height, slices, stacks);
}

void glutWireCone(float base, float height, int slices, int stacks)
{
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSolidCone(base, height, slices, stacks);
	glPopAttrib();
}

void glutSolidCylinder(float rad, float height, int slices, int stacks)
{
	draw_cylinder(rad, rad, height, slices, stacks);
}

void glutWireCylinder(float rad, float height, int slices, int stacks)
{
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSolidCylinder(rad, height, slices, stacks);
	glPopAttrib();
}

void glutSolidTorus(float inner_rad, float outer_rad, int sides, int rings)
{
	int i, j, k, gray;
	float x, y, z, s, t, u, v, phi, theta, sintheta, costheta, sinphi, cosphi;
	float du = 1.0f / (float)rings;
	float dv = 1.0f / (float)sides;

	glBegin(GL_QUADS);
	for(i=0; i<rings; i++) {
		u = i * du;
		for(j=0; j<sides; j++) {
			v = j * dv;
			for(k=0; k<4; k++) {
				gray = k ^ (k >> 1);
				s = gray & 1 ? u + du : u;
				t = gray & 2 ? v + dv : v;
				theta = s * PI * 2.0f;
				phi = t * PI * 2.0f;
				mglut_sincosf(theta, &sintheta, &costheta);
				mglut_sincosf(phi, &sinphi, &cosphi);
				x = sintheta * sinphi;
				y = costheta * sinphi;
				z = cosphi;

				glColor3f(s, t, 1);
				glTexCoord2f(s, t);
				glNormal3f(x, y, z);

				x = x * inner_rad + sintheta * outer_rad;
				y = y * inner_rad + costheta * outer_rad;
				z *= inner_rad;
				glVertex3f(x, y, z);
			}
		}
	}
	glEnd();
}

void glutWireTorus(float inner_rad, float outer_rad, int sides, int rings)
{
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSolidTorus(inner_rad, outer_rad, sides, rings);
	glPopAttrib();
}

void glutSolidTeapot(float size)
{
}

void glutWireTeapot(float size)
{
}



/* --------------- UNIX/X11 implementation ----------------- */
#ifdef BUILD_X11
static void handle_event(XEvent *ev);

void glutMainLoopEvent(void)
{
	XEvent ev;

	if(!cb_display) {
		panic("display callback not set");
	}

	if(!upd_pending && !cb_idle) {
		XNextEvent(dpy, &ev);
		handle_event(&ev);
		if(quit) return;
	}
	while(XPending(dpy)) {
		XNextEvent(dpy, &ev);
		handle_event(&ev);
		if(quit) return;
	}

	if(cb_idle) {
		cb_idle();
	}

	if(upd_pending && mapped) {
		upd_pending = 0;
		cb_display();
	}
}

static KeySym translate_keysym(KeySym sym)
{
	switch(sym) {
	case XK_Escape:
		return 27;
	case XK_BackSpace:
		return '\b';
	case XK_Linefeed:
		return '\r';
	case XK_Return:
		return '\n';
	case XK_Delete:
		return 127;
	case XK_Tab:
		return '\t';
	default:
		break;
	}
	return sym;
}

static void handle_event(XEvent *ev)
{
	KeySym sym;

	switch(ev->type) {
	case MapNotify:
		mapped = 1;
		break;
	case UnmapNotify:
		mapped = 0;
		break;
	case ConfigureNotify:
		if(cb_reshape && (ev->xconfigure.width != win_width || ev->xconfigure.height != win_height)) {
			win_width = ev->xconfigure.width;
			win_height = ev->xconfigure.height;
			cb_reshape(ev->xconfigure.width, ev->xconfigure.height);
		}
		break;

	case ClientMessage:
		if(ev->xclient.message_type == xa_wm_proto) {
			if(ev->xclient.data.l[0] == xa_wm_del_win) {
				quit = 1;
			}
		}
		break;

	case Expose:
		upd_pending = 1;
		break;

	case KeyPress:
	case KeyRelease:
		modstate = ev->xkey.state & (ShiftMask | ControlMask | Mod1Mask);
		if(!(sym = XLookupKeysym(&ev->xkey, 0))) {
			break;
		}
		sym = translate_keysym(sym);
		if(sym < 256) {
			if(ev->type == KeyPress) {
				if(cb_keydown) cb_keydown((unsigned char)sym, ev->xkey.x, ev->xkey.y);
			} else {
				if(cb_keyup) cb_keyup((unsigned char)sym, ev->xkey.x, ev->xkey.y);
			}
		} else {
			if(ev->type == KeyPress) {
				if(cb_skeydown) cb_skeydown(sym, ev->xkey.x, ev->xkey.y);
			} else {
				if(cb_skeyup) cb_skeyup(sym, ev->xkey.x, ev->xkey.y);
			}
		}
		break;

	case ButtonPress:
	case ButtonRelease:
		modstate = ev->xbutton.state & (ShiftMask | ControlMask | Mod1Mask);
		if(cb_mouse) {
			int bn = ev->xbutton.button - Button1;
			cb_mouse(bn, ev->type == ButtonPress ? GLUT_DOWN : GLUT_UP,
					ev->xbutton.x, ev->xbutton.y);
		}
		break;

	case MotionNotify:
		if(ev->xmotion.state & (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)) {
			if(cb_motion) cb_motion(ev->xmotion.x, ev->xmotion.y);
		} else {
			if(cb_passive) cb_passive(ev->xmotion.x, ev->xmotion.y);
		}
		break;

	case VisibilityNotify:
		if(cb_vis) {
			cb_vis(ev->xvisibility.state == VisibilityFullyObscured ? GLUT_NOT_VISIBLE : GLUT_VISIBLE);
		}
		break;
	case EnterNotify:
		if(cb_entry) cb_entry(GLUT_ENTERED);
		break;
	case LeaveNotify:
		if(cb_entry) cb_entry(GLUT_LEFT);
		break;
	}
}

void glutSwapBuffers(void)
{
	glXSwapBuffers(dpy, win);
}

void glutPositionWindow(int x, int y)
{
	XMoveWindow(dpy, win, x, y);
}

void glutReshapeWindow(int xsz, int ysz)
{
	XResizeWindow(dpy, win, xsz, ysz);
}

void glutFullScreen(void)
{
	/* TODO */
}

void glutSetWindowTitle(const char *title)
{
	XTextProperty tprop;
	if(!XStringListToTextProperty((char**)&title, 1, &tprop)) {
		return;
	}
	XSetWMName(dpy, win, &tprop);
	XFree(tprop.value);
}

void glutSetIconTitle(const char *title)
{
	XTextProperty tprop;
	if(!XStringListToTextProperty((char**)&title, 1, &tprop)) {
		return;
	}
	XSetWMIconName(dpy, win, &tprop);
	XFree(tprop.value);
}

void glutSetCursor(int cidx)
{
	Cursor cur = None;

	switch(cidx) {
	case GLUT_CURSOR_LEFT_ARROW:
		cur = XCreateFontCursor(dpy, XC_left_ptr);
		break;
	case GLUT_CURSOR_INHERIT:
		break;
	case GLUT_CURSOR_NONE:
		/* TODO */
	default:
		return;
	}

	XDefineCursor(dpy, win, cur);
	cur_cursor = cidx;
}

static XVisualInfo *choose_visual(unsigned int mode)
{
	XVisualInfo *vi;
	int attr[32];
	int *aptr = attr;
	int *samples = 0;

	if(mode & GLUT_DOUBLE) {
		*aptr++ = GLX_DOUBLEBUFFER;
	}

	if(mode & GLUT_INDEX) {
		*aptr++ = GLX_BUFFER_SIZE;
		*aptr++ = 1;
	} else {
		*aptr++ = GLX_RGBA;
		*aptr++ = GLX_RED_SIZE; *aptr++ = 4;
		*aptr++ = GLX_GREEN_SIZE; *aptr++ = 4;
		*aptr++ = GLX_BLUE_SIZE; *aptr++ = 4;
	}
	if(mode & GLUT_ALPHA) {
		*aptr++ = GLX_ALPHA_SIZE;
		*aptr++ = 4;
	}
	if(mode & GLUT_DEPTH) {
		*aptr++ = GLX_DEPTH_SIZE;
		*aptr++ = 16;
	}
	if(mode & GLUT_STENCIL) {
		*aptr++ = GLX_STENCIL_SIZE;
		*aptr++ = 1;
	}
	if(mode & GLUT_ACCUM) {
		*aptr++ = GLX_ACCUM_RED_SIZE; *aptr++ = 1;
		*aptr++ = GLX_ACCUM_GREEN_SIZE; *aptr++ = 1;
		*aptr++ = GLX_ACCUM_BLUE_SIZE; *aptr++ = 1;
	}
	if(mode & GLUT_STEREO) {
		*aptr++ = GLX_STEREO;
	}
	if(mode & GLUT_SRGB) {
		*aptr++ = GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB;
	}
	if(mode & GLUT_MULTISAMPLE) {
		*aptr++ = GLX_SAMPLE_BUFFERS_ARB;
		*aptr++ = 1;
		*aptr++ = GLX_SAMPLES_ARB;
		samples = aptr;
		*aptr++ = 32;
	}
	*aptr++ = None;

	if(!samples) {
		return glXChooseVisual(dpy, scr, attr);
	}
	while(!(vi = glXChooseVisual(dpy, scr, attr)) && *samples) {
		*samples >>= 1;
		if(!*samples) {
			aptr[-3] = None;
		}
	}
	return vi;
}

static void create_window(const char *title)
{
	XSetWindowAttributes xattr;
	XVisualInfo *vi;
	unsigned int xattr_mask;
	unsigned int mode = init_mode;

	if(!(vi = choose_visual(mode))) {
		mode &= ~GLUT_SRGB;
		if(!(vi = choose_visual(mode))) {
			panic("Failed to find compatible visual\n");
		}
	}

	if(!(ctx = glXCreateContext(dpy, vi, 0, True))) {
		XFree(vi);
		panic("Failed to create OpenGL context\n");
	}

	glXGetConfig(dpy, vi, GLX_RED_SIZE, &ctx_info.rsize);
	glXGetConfig(dpy, vi, GLX_GREEN_SIZE, &ctx_info.gsize);
	glXGetConfig(dpy, vi, GLX_BLUE_SIZE, &ctx_info.bsize);
	glXGetConfig(dpy, vi, GLX_ALPHA_SIZE, &ctx_info.asize);
	glXGetConfig(dpy, vi, GLX_DEPTH_SIZE, &ctx_info.zsize);
	glXGetConfig(dpy, vi, GLX_STENCIL_SIZE, &ctx_info.ssize);
	glXGetConfig(dpy, vi, GLX_DOUBLEBUFFER, &ctx_info.dblbuf);
	glXGetConfig(dpy, vi, GLX_STEREO, &ctx_info.stereo);
	glXGetConfig(dpy, vi, GLX_SAMPLES_ARB, &ctx_info.samples);
	glXGetConfig(dpy, vi, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, &ctx_info.srgb);

	xattr.background_pixel = BlackPixel(dpy, scr);
	xattr.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	xattr_mask = CWBackPixel | CWColormap;
	if(!(win = XCreateWindow(dpy, root, init_x, init_y, init_width, init_height, 0,
			vi->depth, InputOutput, vi->visual, xattr_mask, &xattr))) {
		XFree(vi);
		glXDestroyContext(dpy, ctx);
		panic("Failed to create window\n");
	}
	XFree(vi);

	XSelectInput(dpy, win, evmask);

	glutSetWindowTitle(title);
	glutSetIconTitle(title);
	XSetWMProtocols(dpy, win, &xa_wm_del_win, 1);
	XMapWindow(dpy, win);

	glXMakeCurrent(dpy, win, ctx);
}

static void get_window_pos(int *x, int *y)
{
	XWindowAttributes wattr;
	XGetWindowAttributes(dpy, win, &wattr);
	*x = wattr.x;
	*y = wattr.y;
}

static void get_window_size(int *w, int *h)
{
	XWindowAttributes wattr;
	XGetWindowAttributes(dpy, win, &wattr);
	*w = wattr.width;
	*h = wattr.height;
}

static void get_screen_size(int *scrw, int *scrh)
{
	XWindowAttributes wattr;
	XGetWindowAttributes(dpy, root, &wattr);
	*scrw = wattr.width;
	*scrh = wattr.height;
}
#endif	/* BUILD_X11 */


/* --------------- windows implementation ----------------- */
#ifdef BUILD_WIN32
static int reshape_pending;

static void update_modkeys(void);
static int translate_vkey(int vkey);
static void handle_mbutton(int bn, int st, WPARAM wparam, LPARAM lparam);

#ifdef MINIGLUT_WINMAIN
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, char *cmdline, int showcmd)
{
	int argc = 1;
	char *argv[] = { "miniglut.exe", 0 };
	return main(argc, argv);
}
#endif

void glutMainLoopEvent(void)
{
	MSG msg;

	if(!cb_display) {
		panic("display callback not set");
	}

	if(reshape_pending && cb_reshape) {
		reshape_pending = 0;
		get_window_size(&win_width, &win_height);
		cb_reshape(win_width, win_height);
	}

	if(!upd_pending && !cb_idle) {
		GetMessage(&msg, 0, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(quit) return;
	}
	while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(quit) return;
	}

	if(cb_idle) {
		cb_idle();
	}

	if(upd_pending && mapped) {
		upd_pending = 0;
		cb_display();
	}
}

void glutSwapBuffers(void)
{
	SwapBuffers(dc);
}

void glutPositionWindow(int x, int y)
{
	RECT rect;
	GetWindowRect(win, &rect);
	MoveWindow(win, x, y, rect.right - rect.left, rect.bottom - rect.top, 1);
}

void glutReshapeWindow(int xsz, int ysz)
{
	RECT rect;
	GetWindowRect(win, &rect);
	MoveWindow(win, rect.left, rect.top, xsz, ysz, 1);
}

void glutFullScreen(void)
{
	/* TODO */
}

void glutSetWindowTitle(const char *title)
{
	SetWindowText(win, title);
}

void glutSetIconTitle(const char *title)
{
}

void glutSetCursor(int cidx)
{
	switch(cidx) {
	case GLUT_CURSOR_NONE:
		ShowCursor(0);
		break;
	case GLUT_CURSOR_INHERIT:
	case GLUT_CURSOR_LEFT_ARROW:
	default:
		SetCursor(LoadCursor(0, IDC_ARROW));
		ShowCursor(1);
	}
}


static void create_window(const char *title)
{
	int pixfmt;
	PIXELFORMATDESCRIPTOR pfd = {0};

	if(!(win = CreateWindow("MiniGLUT", title, WS_OVERLAPPEDWINDOW, init_x, init_y,
				init_width, init_height, 0, 0, hinst, 0))) {
		panic("Failed to create window\n");
	}
	dc = GetDC(win);

	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	if(init_mode & GLUT_STEREO) {
		pfd.dwFlags |= PFD_STEREO;
	}
	pfd.iPixelType = init_mode & GLUT_INDEX ? PFD_TYPE_COLORINDEX : PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	if(init_mode & GLUT_ALPHA) {
		pfd.cAlphaBits = 8;
	}
	if(init_mode & GLUT_ACCUM) {
		pfd.cAccumBits = 24;
	}
	if(init_mode & GLUT_DEPTH) {
		pfd.cDepthBits = 24;
	}
	if(init_mode & GLUT_STENCIL) {
		pfd.cStencilBits = 8;
	}
	pfd.iLayerType = PFD_MAIN_PLANE;

	if(!(pixfmt = ChoosePixelFormat(dc, &pfd))) {
		panic("Failed to find suitable pixel format\n");
	}
	if(!SetPixelFormat(dc, pixfmt, &pfd)) {
		panic("Failed to set the selected pixel format\n");
	}
	if(!(ctx = wglCreateContext(dc))) {
		panic("Failed to create the OpenGL context\n");
	}
	wglMakeCurrent(dc, ctx);

	DescribePixelFormat(dc, pixfmt, sizeof pfd, &pfd);
	ctx_info.rsize = pfd.cRedBits;
	ctx_info.gsize = pfd.cGreenBits;
	ctx_info.bsize = pfd.cBlueBits;
	ctx_info.asize = pfd.cAlphaBits;
	ctx_info.zsize = pfd.cDepthBits;
	ctx_info.ssize = pfd.cStencilBits;
	ctx_info.dblbuf = pfd.dwFlags & PFD_DOUBLEBUFFER ? 1 : 0;
	ctx_info.samples = 1;	/* TODO */
	ctx_info.srgb = 0;		/* TODO */

	ShowWindow(win, 1);
	upd_pending = 1;
	reshape_pending = 1;
}

static HRESULT CALLBACK handle_message(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
	static int mouse_x, mouse_y;
	int key;

	switch(msg) {
	case WM_CLOSE:
		if(win) DestroyWindow(win);
		break;

	case WM_DESTROY:
		wglMakeCurrent(dc, 0);
		wglDeleteContext(ctx);
		quit = 1;
		PostQuitMessage(0);
		break;

	case WM_PAINT:
		upd_pending = 1;
		ValidateRect(win, 0);
		break;

	case WM_SIZE:
		win_width = lparam & 0xffff;
		win_height = lparam >> 16;
		if(cb_reshape) {
			reshape_pending = 0;
			cb_reshape(win_width, win_height);
		}
		break;

	case WM_SHOWWINDOW:
		mapped = wparam;
		if(cb_vis) cb_vis(mapped ? GLUT_VISIBLE : GLUT_NOT_VISIBLE);
		break;

	case WM_KEYDOWN:
		update_modkeys();
		key = translate_vkey(wparam);
		if(key < 256) {
			if(cb_keydown) {
				cb_keydown((unsigned char)key, mouse_x, mouse_y);
			}
		} else {
			if(cb_skeydown) {
				cb_skeydown(key, mouse_x, mouse_y);
			}
		}
		break;

	case WM_KEYUP:
		update_modkeys();
		key = translate_vkey(wparam);
		if(key < 256) {
			if(cb_keyup) {
				cb_keyup((unsigned char)key, mouse_x, mouse_y);
			}
		} else {
			if(cb_skeyup) {
				cb_skeyup(key, mouse_x, mouse_y);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		handle_mbutton(0, 1, wparam, lparam);
		break;
	case WM_MBUTTONDOWN:
		handle_mbutton(1, 1, wparam, lparam);
		break;
	case WM_RBUTTONDOWN:
		handle_mbutton(2, 1, wparam, lparam);
		break;
	case WM_LBUTTONUP:
		handle_mbutton(0, 0, wparam, lparam);
		break;
	case WM_MBUTTONUP:
		handle_mbutton(1, 0, wparam, lparam);
		break;
	case WM_RBUTTONUP:
		handle_mbutton(2, 0, wparam, lparam);
		break;

	case WM_MOUSEMOVE:
		if(wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
			if(cb_motion) cb_motion(lparam & 0xffff, lparam >> 16);
		} else {
			if(cb_passive) cb_passive(lparam & 0xffff, lparam >> 16);
		}
		break;

	default:
		return DefWindowProc(win, msg, wparam, lparam);
	}

	return 0;
}

static void update_modkeys(void)
{
	if(GetKeyState(VK_SHIFT)) {
		modstate |= GLUT_ACTIVE_SHIFT;
	} else {
		modstate &= ~GLUT_ACTIVE_SHIFT;
	}
	if(GetKeyState(VK_CONTROL)) {
		modstate |= GLUT_ACTIVE_CTRL;
	} else {
		modstate &= ~GLUT_ACTIVE_CTRL;
	}
	if(GetKeyState(VK_MENU)) {
		modstate |= GLUT_ACTIVE_ALT;
	} else {
		modstate &= ~GLUT_ACTIVE_ALT;
	}
}

static int translate_vkey(int vkey)
{
	switch(vkey) {
	case VK_PRIOR: return GLUT_KEY_PAGE_UP;
	case VK_NEXT: return GLUT_KEY_PAGE_DOWN;
	case VK_END: return GLUT_KEY_END;
	case VK_HOME: return GLUT_KEY_HOME;
	case VK_LEFT: return GLUT_KEY_LEFT;
	case VK_UP: return GLUT_KEY_UP;
	case VK_RIGHT: return GLUT_KEY_RIGHT;
	case VK_DOWN: return GLUT_KEY_DOWN;
	default:
		if(vkey >= VK_F1 && vkey <= VK_F12) {
			return vkey - VK_F1 + GLUT_KEY_F1;
		}
	}
	return vkey;
}

static void handle_mbutton(int bn, int st, WPARAM wparam, LPARAM lparam)
{
	int x, y;

	update_modkeys();

	if(cb_mouse) {
		x = lparam & 0xffff;
		y = lparam >> 16;
		cb_mouse(bn, st ? GLUT_DOWN : GLUT_UP, x, y);
	}
}

static void get_window_pos(int *x, int *y)
{
	RECT rect;
	GetWindowRect(win, &rect);
	*x = rect.left;
	*y = rect.top;
}

static void get_window_size(int *w, int *h)
{
	RECT rect;
	GetClientRect(win, &rect);
	*w = rect.right - rect.left;
	*h = rect.bottom - rect.top;
}

static void get_screen_size(int *scrw, int *scrh)
{
	*scrw = GetSystemMetrics(SM_CXSCREEN);
	*scrh = GetSystemMetrics(SM_CYSCREEN);
}
#endif	/* BUILD_WIN32 */

#if defined(__unix__) || defined(__APPLE__)
#include <sys/time.h>

#ifdef MINIGLUT_USE_LIBC
#define sys_gettimeofday(tv, tz)	gettimeofday(tv, tz)
#else
static int sys_gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

static long get_msec(void)
{
	static struct timeval tv0;
	struct timeval tv;

	sys_gettimeofday(&tv, 0);
	if(tv0.tv_sec == 0 && tv0.tv_usec == 0) {
		tv0 = tv;
		return 0;
	}
	return (tv.tv_sec - tv0.tv_sec) * 1000 + (tv.tv_usec - tv0.tv_usec) / 1000;
}
#endif
#ifdef _WIN32
static long get_msec(void)
{
	static long t0;
	long tm;

#ifdef MINIGLUT_NO_WINMM
	tm = GetTickCount();
#else
	tm = timeGetTime();
#endif
	if(!t0) {
		t0 = tm;
		return 0;
	}
	return tm - t0;
}
#endif

static void panic(const char *msg)
{
	const char *end = msg;
	while(*end) end++;
	sys_write(2, msg, end - msg);
	sys_exit(1);
}


#ifdef MINIGLUT_USE_LIBC
static void sys_exit(int status)
{
	exit(status);
}

static int sys_write(int fd, const void *buf, int count)
{
	return write(fd, buf, count);
}

static int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return gettimeofday(tv, tz);
}

#else	/* !MINIGLUT_USE_LIBC */

#ifdef __GNUC__
static void mglut_sincosf(float angle, float *sptr, float *cptr)
{
	asm volatile(
		"flds %2\n\t"
		"fsincos\n\t"
		"fstps %1\n\t"
		"fstps %0\n\t"
		: "=m"(*sptr), "=m"(*cptr)
		: "m"(angle)
	);
}

static float mglut_atan(float x)
{
	float res;
	asm volatile(
		"flds %1\n\t"
		"fld1\n\t"
		"fpatan\n\t"
		"fstps %0\n\t"
		: "=m"(res)
		: "m"(x)
	);
	return res;
}
#endif

#ifdef _MSC_VER
static void mglut_sincosf(float angle, float *sptr, float *cptr)
{
	float s, c;
	__asm {
		fld angle
		fsincos
		fstp c
		fstp s
	}
	*sptr = s;
	*cptr = c;
}

static float mglut_atan(float x)
{
	float res;
	__asm {
		fld x
		fld1
		fpatan
		fstp res
	}
	return res;
}
#endif

#ifdef __WATCOMC__
#pragma aux mglut_sincosf = \
	"fsincos" \
	"fstp dword ptr [edx]" \
	"fstp dword ptr [eax]" \
	parm[8087][eax][edx]	\
	modify[8087];

#pragma aux mglut_atan = \
	"fld1" \
	"fpatan" \
	parm[8087] \
	value[8087] \
	modify [8087];
#endif

#ifdef __linux__

#ifdef __x86_64__
static void sys_exit(int status)
{
	asm volatile(
		"syscall\n\t"
		:: "a"(60), "D"(status));
}
static int sys_write(int fd, const void *buf, int count)
{
	long res;
	asm volatile(
		"syscall\n\t"
		: "=a"(res)
		: "a"(1), "D"(fd), "S"(buf), "d"(count));
	return res;
}
static int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int res;
	asm volatile(
		"syscall\n\t"
		: "=a"(res)
		: "a"(96), "D"(tv), "S"(tz));
	return res;
}
#endif
#ifdef __i386__
static void sys_exit(int status)
{
	asm volatile(
		"int $0x80\n\t"
		:: "a"(1), "b"(status));
}
static int sys_write(int fd, const void *buf, int count)
{
	int res;
	asm volatile(
		"int $0x80\n\t"
		: "=a"(res)
		: "a"(4), "b"(fd), "c"(buf), "d"(count));
	return res;
}
static int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int res;
	asm volatile(
		"int $0x80\n\t"
		: "=a"(res)
		: "a"(78), "b"(tv), "c"(tz));
	return res;
}
#endif

#endif	/* __linux__ */

#ifdef _WIN32
static void sys_exit(int status)
{
	ExitProcess(status);
}
static int sys_write(int fd, const void *buf, int count)
{
	unsigned long wrsz = 0;

	HANDLE out = GetStdHandle(fd == 1 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	if(!WriteFile(out, buf, count, &wrsz, 0)) {
		return -1;
	}
	return wrsz;
}
#endif	/* _WIN32 */

#endif	/* !MINIGLUT_USE_LIBC */
