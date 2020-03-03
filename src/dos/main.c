#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "keyb.h"
#include "timer.h"
#include "gfx.h"
#include "logger.h"
#include "cdpmi.h"

static void draw(void);

static struct video_mode *vmode;


int main(int argc, char **argv)
{
	struct video_mode *vmodes;
	int vmidx;

	init_logger("game.log");

	init_timer(100);
	kb_init(32);

	if(init_video() == -1) {
		return 1;
	}
	vmodes = video_modes();

	if((vmidx = match_video_mode(640, 480, 16)) == -1) {
		return 1;
	}
	if(!(vmem = set_video_mode(vmidx, 1))) {
		return 1;
	}
	vmode = vmodes + vmidx;

	fb_width = vmode->xsz;
	fb_height = vmode->ysz;
	fb_size = (vmode->xsz * vmode->bpp / 8) * vmode->ysz;

	reset_timer();

	for(;;) {
		int key;
		while((key = kb_getkey()) != -1) {
			if(key == 27) goto break_evloop;
		}

		time_msec = get_msec();
		draw();
	}

break_evloop:
	set_text_mode();
	cleanup_video();
	kb_shutdown();
	return 0;
}

static void draw(void)
{
	int i, j;
	uint16_t *pptr = vmem;

	for(i=0; i<fb_height; i++) {
		for(j=0; j<fb_width; j++) {
			int chess = ((i >> 4) & 1) == ((j >> 4) & 1);
			*pptr++ = chess ? 0xff00 : 0x00ff;
		}
	}
}

void swap_buffers(void *pixels)
{
	wait_vsync();
	memcpy(vmem, pixels, fb_size);
}
