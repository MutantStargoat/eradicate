#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <SDL/SDL.h>
#include "game.h"
#include "timer.h"

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


int main(int argc, char **argv)
{
	int s;
	char *env;
	void *fb_buf;

	if((env = getenv("FBSCALE")) && (s = atoi(env))) {
		fbscale = s;
		printf("Framebuffer scaling x%d\n", fbscale);
	}

	xsz = FB_WIDTH * fbscale;
	ysz = FB_HEIGHT * fbscale;
	fb_width = xsz;
	fb_height = ysz;

	fb_size = FB_WIDTH * FB_HEIGHT * FB_BPP / 8;
	if(!(fb_buf = malloc(fb_size + FB_WIDTH * 4))) {
		fprintf(stderr, "failed to allocate virtual framebuffer\n");
		return 1;
	}
	fb_pixels = (uint16_t*)((char*)fb_buf + FB_WIDTH * 2);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);
	if(!(fbsurf = SDL_SetVideoMode(xsz, ysz, FB_BPP, sdl_flags))) {
		fprintf(stderr, "failed to set video mode %dx%d %dbpp\n", FB_WIDTH, FB_HEIGHT, FB_BPP);
		free(fb_pixels);
		SDL_Quit();
		return 1;
	}
	SDL_WM_SetCaption("eradicate/SDL", 0);
	SDL_ShowCursor(0);

	time_msec = 0;
	if(init(argc, argv) == -1) {
		free(fb_pixels);
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

		time_msec = get_msec();
		draw();
	}

break_evloop:
	cleanup();
	SDL_Quit();
	return 0;
}

void game_quit(void)
{
	quit = 1;
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

	if(vsync) {
		wait_vsync();
	}

	if(SDL_MUSTLOCK(fbsurf)) {
		SDL_LockSurface(fbsurf);
	}

	sptr = pixels;
	dptr = (unsigned short*)fbsurf->pixels + (fbsurf->w - xsz) / 2;
	for(i=0; i<FB_HEIGHT; i++) {
		for(j=0; j<FB_WIDTH; j++) {
			int x, y;
			unsigned short pixel = *sptr++;

			for(y=0; y<fbscale; y++) {
				for(x=0; x<fbscale; x++) {
					dptr[y * fbsurf->w + x] = pixel;
				}
			}
			dptr += fbscale;
		}
		dptr += (fbsurf->w - FB_WIDTH) * fbscale;
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

static void handle_event(SDL_Event *ev)
{
	int key;

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
		if(key_event) {
			key = sdlkey_to_gamekey(ev->key.keysym.sym, ev->key.keysym.mod);
			key_event(key, ev->key.state == SDL_PRESSED ? 1 : 0);
		} else {
			if(ev->key.keysym.sym == SDLK_ESCAPE) {
				quit = 1;
			}
		}
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
