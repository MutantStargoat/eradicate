#include "game.h"

int fb_width, fb_height;
long fb_size;
void *fb_pixels, *vmem;

long time_msec;

void (*swap_buffers)(void*);


int game_init(int argc, char **argv)
{
	return 0;
}

void game_cleanup(void)
{
}

void game_draw(void)
{
}

void game_keyboard(int key, int press)
{
}
