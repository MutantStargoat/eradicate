#include "game.h"
#include "screens.h"

int fb_width, fb_height;
long fb_size;
void *fb_pixels, *vmem;

long time_msec;

void (*draw)(void);
void (*key_event)(int key, int pressed);


int init(int argc, char **argv)
{
	if(intro_init() == -1) {
		return -1;
	}
	if(menu_init() == -1) {
		return -1;
	}

	draw = intro_draw;
	key_event = intro_keyb;
	return 0;
}

void cleanup(void)
{
	intro_cleanup();
	menu_cleanup();
}
