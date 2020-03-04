#include "game.h"
#include "menuscr.h"

int fb_width, fb_height;
long fb_size;
void *fb_pixels, *vmem;

long time_msec;

void (*draw)(void);
void (*key_event)(int key, int pressed);


int init(int argc, char **argv)
{
	if(menu_init() == -1) {
		return -1;
	}

	draw = menu_draw;
	key_event = menu_keyb;
	return 0;
}

void cleanup(void)
{
	menu_cleanup();
}
