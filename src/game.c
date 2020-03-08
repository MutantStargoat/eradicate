#include "game.h"
#include "screens.h"
#include "sprite.h"

int fb_width, fb_height;
long fb_size;
uint16_t *fb_pixels;

long time_msec;

void (*draw)(void);
void (*key_event)(int key, int pressed);

static struct sprites dbgfont;


int init(int argc, char **argv)
{
	if(load_sprites(&dbgfont, "data/dbgfont.spr") == -1) {
		return -1;
	}
	if(intro_init() == -1) {
		return -1;
	}
	if(menu_init() == -1) {
		return -1;
	}

	intro_start();
	return 0;
}

void cleanup(void)
{
	intro_cleanup();
	menu_cleanup();
}

void dbg_print(void *fb, int x, int y, const char *str)
{
	uint16_t *dest = (uint16_t*)fb + y * fb_width + x;

	while(*str) {
		int c = *str++;

		if(c > ' ' && c < 128) {
			draw_sprite(dest, fb_width * 2, &dbgfont, c - ' ');
		}
		dest += 8;
	}
}

