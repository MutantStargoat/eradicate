#include "game.h"
#include "screens.h"

int fb_width, fb_height;
long fb_size;
uint16_t *fb_pixels;

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

	intro_start();
	return 0;
}

void cleanup(void)
{
	intro_cleanup();
	menu_cleanup();
}

void cs_puts_font(cs_font_func csfont, int sz, void *fb, int x, int y, const char *str)
{
	while(*str) {
		int c = *str++;

		if(c > ' ' && c < 128) {
			csfont(fb, x, y, c - ' ');
		}
		x += sz;
	}
}

