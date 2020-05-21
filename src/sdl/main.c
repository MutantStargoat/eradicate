#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <SDL.h>
#include "game.h"
#include "gfx.h"
#include "timer.h"
#include "joy.h"
#include "input.h"
#include "audio.h"

#define FB_WIDTH	640
#define FB_HEIGHT	480

static void handle_event(SDL_Event *ev);
static void toggle_fullscreen(void);

static int sdlkey_to_gamekey(int sdlkey, unsigned int mod);


static int quit;
static SDL_Surface *fbsurf;

static int fbscale = 1;
static int xsz, ysz;
static unsigned int sdl_flags = SDL_SWSURFACE;

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

static SDL_Joystick *joy;
static int joy_numaxes, joy_numbn;
int have_joy;
unsigned int joy_bnstate, joy_bndiff, joy_bnpress;
static unsigned int joy_bnprev;
int16_t joy_pos[2];


int main(int argc, char **argv)
{
	int s;
	char *env;

	if((env = getenv("FBSCALE")) && (s = atoi(env))) {
		fbscale = s;
		printf("Framebuffer scaling x%d\n", fbscale);
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE);
	if(!set_video_mode(match_video_mode(FB_WIDTH, FB_HEIGHT, FB_BPP), 1)) {
		return 1;
	}

	SDL_WM_SetCaption("eradicate/SDL", 0);
	SDL_ShowCursor(0);

	if(au_init() == -1) {
		return 1;
	}

	if((joy = SDL_JoystickOpen(0))) {
		joy_numaxes = SDL_JoystickNumAxes(joy);
		joy_numbn = SDL_JoystickNumButtons(joy);
		printf("Using joystick: %s (%d axes, %d buttons)\n", SDL_JoystickName(0),
				joy_numaxes, joy_numbn);
	}

	time_msec = 0;
	if(init(argc, argv) == -1) {
		resizefb(0, 0, 0);
		SDL_Quit();
		return 1;
	}

	reset_timer();

	while(!quit) {
		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			handle_event(&ev);
			if(quit) goto break_evloop;
		}

		/* update input state from keyboard and joystick */
		inp_update();

		time_msec = get_msec();
		draw();
	}

break_evloop:
	cleanup();
	resizefb(0, 0, 0);
	au_shutdown();
	if(joy) SDL_JoystickClose(joy);
	SDL_Quit();
	return 0;
}

void game_quit(void)
{
	quit = 1;
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

void *set_video_mode(int idx, int nbuf)
{
	struct video_mode *vm = vmodes + idx;

	if(cur_vmode == vm) {
		return vmem;
	}

	if(!(fbsurf = SDL_SetVideoMode(vm->xsz * fbscale, vm->ysz * fbscale, vm->bpp, sdl_flags))) {
		fprintf(stderr, "failed to set video mode %dx%d %dbpp\n", vm->xsz, vm->ysz, vm->bpp);
		return 0;
	}

	xsz = vm->xsz * fbscale;
	ysz = vm->ysz * fbscale;

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
	unsigned long start = SDL_GetTicks();
	unsigned long until = (start | 0xf) + 1;
	while(SDL_GetTicks() <= until);
}

void blit_frame(void *pixels, int vsync)
{
	int i, j;
	unsigned short *sptr, *dptr;

	if(show_fps) dbg_fps(pixels);

	if(vsync) {
		wait_vsync();
	}

	if(SDL_MUSTLOCK(fbsurf)) {
		SDL_LockSurface(fbsurf);
	}

	sptr = pixels;
	dptr = (unsigned short*)fbsurf->pixels + (fbsurf->w - xsz) / 2;
	for(i=0; i<fb_height; i++) {
		for(j=0; j<fb_width; j++) {
			int x, y;
			unsigned short pixel = *sptr++;

			for(y=0; y<fbscale; y++) {
				for(x=0; x<fbscale; x++) {
					dptr[y * fbsurf->w + x] = pixel;
				}
			}
			dptr += fbscale;
		}
		dptr += (fbsurf->w - fb_width) * fbscale;
	}

	if(SDL_MUSTLOCK(fbsurf)) {
		SDL_UnlockSurface(fbsurf);
	}
	SDL_Flip(fbsurf);
}

/*
static int bnmask(int sdlbn)
{
	switch(sdlbn) {
	case SDL_BUTTON_LEFT:
		return MOUSE_BN_LEFT;
	case SDL_BUTTON_RIGHT:
		return MOUSE_BN_RIGHT;
	case SDL_BUTTON_MIDDLE:
		return MOUSE_BN_MIDDLE;
	default:
		break;
	}
	return 0;
}
*/

int joy_detect(void)
{
	have_joy = joy != 0;
	return have_joy;
}

void joy_update(void)
{
	int i, val;

	SDL_JoystickUpdate();

	joy_bnprev = joy_bnstate;
	joy_bnstate = 0;

	if(joy_numaxes >= 2) {
		val = SDL_JoystickGetAxis(joy, 0);
		joy_pos[0] = val;
		if(val < -8192) {
			joy_bnstate |= JOY_LEFT;
		} else if(val > 8192) {
			joy_bnstate |= JOY_RIGHT;
		}
		val = SDL_JoystickGetAxis(joy, 1);
		joy_pos[1] = val;
		if(val < -8192) {
			joy_bnstate |= JOY_UP;
		} else if(val > 8192) {
			joy_bnstate |= JOY_DOWN;
		}
	}

	for(i=0; i<joy_numbn; i++) {
		if(SDL_JoystickGetButton(joy, i)) {
			joy_bnstate |= JOY_BN0 << i;
		}
	}

	joy_bndiff = joy_bnstate ^ joy_bnprev;
	joy_bnpress = joy_bnstate & joy_bndiff;
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

static void handle_event(SDL_Event *ev)
{
	int key, pressed;

	switch(ev->type) {
	case SDL_QUIT:
		quit = 1;
		break;

	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if(ev->key.keysym.sym == SDLK_RETURN && (SDL_GetModState() & KMOD_ALT) &&
				ev->key.state == SDL_PRESSED) {
			toggle_fullscreen();
			break;
		}
		key = sdlkey_to_gamekey(ev->key.keysym.sym, ev->key.keysym.mod);
		pressed = ev->key.state == SDL_PRESSED ? 1 : 0;
		keystate[key] = pressed;

		game_key(key, pressed);
		break;

		/*
	case SDL_MOUSEMOTION:
		mouse_x = ev->motion.x / fbscale;
		mouse_y = ev->motion.y / fbscale;
		break;

	case SDL_MOUSEBUTTONDOWN:
		mouse_bmask |= bnmask(ev->button.button);
		if(0) {
	case SDL_MOUSEBUTTONUP:
			mouse_bmask &= ~bnmask(ev->button.button);
		}
		mouse_x = ev->button.x / fbscale;
		mouse_y = ev->button.y / fbscale;
		break;
		*/

	default:
		break;
	}
}

static void toggle_fullscreen(void)
{
	SDL_Surface *newsurf;
	unsigned int newflags = sdl_flags ^ SDL_FULLSCREEN;

	if(!(newsurf = SDL_SetVideoMode(xsz, ysz, FB_BPP, newflags))) {
		fprintf(stderr, "failed to go %s\n", newflags & SDL_FULLSCREEN ? "fullscreen" : "windowed");
		return;
	}

	fbsurf = newsurf;
	sdl_flags = newflags;
}

#define SSORG	'\''
#define SSEND	'`'
static char symshift[] = {
	'"', 0, 0, 0, 0, '<', '_', '>', '?',
	')', '!', '@', '#', '$', '%', '^', '&', '*', '(',
	0, ':', 0, '+', 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	'{', '|', '}', 0, 0, '~'
};


static int sdlkey_to_gamekey(int sdlkey, unsigned int mod)
{
	if(sdlkey < 128) {
		if(mod & (KMOD_SHIFT)) {
			if(sdlkey >= 'a' && sdlkey <= 'z') {
				sdlkey = toupper(sdlkey);
			} else if(sdlkey >= SSORG && sdlkey <= SSEND) {
				sdlkey = symshift[sdlkey - SSORG];
			}
		}
		return sdlkey;
	}
	if(sdlkey < 256) return 0;
	return sdlkey - 128;
}
