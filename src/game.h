#ifndef GAME_H_
#define GAME_H_

#include "inttypes.h"
#include "options.h"

#define GAME_CFG_FILE	"game.cfg"
#define FB_BPP	16

extern int fb_width;
extern int fb_height;
extern float fb_aspect;
extern int fb_bpp;
extern int fb_scan_size;
extern long fb_size;
extern void *fb_pixels, *vmem;

extern long time_msec;
extern int show_fps;

extern void (*swap_buffers)(void*);

/* special keys */
enum {
	KB_BACKSP = 8,
	KB_ESC = 27,
	KB_DEL = 127,

	KB_NUM_0, KB_NUM_1, KB_NUM_2, KB_NUM_3, KB_NUM_4,
	KB_NUM_5, KB_NUM_6, KB_NUM_7, KB_NUM_8, KB_NUM_9,
	KB_NUM_DOT, KB_NUM_DIV, KB_NUM_MUL, KB_NUM_MINUS, KB_NUM_PLUS, KB_NUM_ENTER, KB_NUM_EQUALS,
	KB_UP, KB_DOWN, KB_RIGHT, KB_LEFT,
	KB_INSERT, KB_HOME, KB_END, KB_PGUP, KB_PGDN,
	KB_F1, KB_F2, KB_F3, KB_F4, KB_F5, KB_F6,
	KB_F7, KB_F8, KB_F9, KB_F10, KB_F11, KB_F12,
	KB_F13, KB_F14, KB_F15,
	KB_NUMLK, KB_CAPSLK, KB_SCRLK,
	KB_RSHIFT, KB_LSHIFT, KB_RCTRL, KB_LCTRL, KB_RALT, KB_LALT,
	KB_RMETA, KB_LMETA, KB_LSUPER, KB_RSUPER, KB_MODE, KB_COMPOSE,
	KB_HELP, KB_PRINT, KB_SYSRQ, KB_BREAK
};

#ifndef KB_ANY
#define KB_ANY		(-1)
#define KB_ALT		(-2)
#define KB_CTRL		(-3)
#define KB_SHIFT	(-4)
#endif


int init(int argc, char **argv);
void cleanup(void);

int resizefb(int width, int height, int bpp);

extern void (*draw)(void);
extern void (*key_event)(int key, int pressed);
extern void (*input_event)(int inp);

void game_key(int key, int pressed);
void game_quit(void);

void draw_volume_bar(void *fb, int x, int y);

void dbg_print(void *fb, int x, int y, const char *str);
void dbg_printf(void *fb, int x, int y, const char *fmt, ...);
void dbg_fps(void *fb);

int kb_isdown(int key);

#endif	/* GAME_H_ */
