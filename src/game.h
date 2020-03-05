#ifndef GAME_H_
#define GAME_H_

#include "inttypes.h"

#define FB_BPP	16

extern int fb_width;
extern int fb_height;
extern long fb_size;
extern uint16_t *fb_pixels;

extern long time_msec;

extern void (*swap_buffers)(void*);

/* special keys */
enum {
	KB_ESC = 27,
	KB_BACKSP = 127,

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

int init(int argc, char **argv);
void cleanup(void);

extern void (*draw)(void);
extern void (*key_event)(int key, int pressed);

void game_quit(void);


#endif	/* GAME_H_ */
