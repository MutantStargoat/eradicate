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

static void swap_lfb(void *pixels);
static void swap_banked(void *pixels);


int main(int argc, char **argv)
{
	void *fb_buf;
	struct video_mode *vmodes;
	int vmidx, status = 0;

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

	if(vmode->fb_addr) {
		swap_buffers = swap_lfb;
	} else {
		swap_buffers = swap_banked;
	}

	fb_width = vmode->xsz;
	fb_height = vmode->ysz;
	fb_size = vmode->pitch * vmode->ysz;

	if(!(fb_buf = malloc(fb_size + vmode->pitch * 2))) {
		fprintf(stderr, "failed to allocate framebuffer\n");
		status = -1;
		goto break_evloop;
	}
	fb_pixels = (char*)fb_buf + vmode->pitch;

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
	free(fb_buf);
	set_text_mode();
	cleanup_video();
	kb_shutdown();
	return status;
}

static void draw(void)
{
	int i, j;
	uint16_t *pptr = fb_pixels;

	for(i=0; i<fb_height; i++) {
		for(j=0; j<fb_width; j++) {
			int chess = ((i >> 4) & 1) == ((j >> 4) & 1);
			*pptr++ = chess ? 0xff00 : 0x00ff;
		}
	}

	swap_buffers(fb_pixels);
}

static void swap_lfb(void *pixels)
{
	wait_vsync();
	memcpy(vmem, pixels, fb_size);
}

static void swap_banked(void *pixels)
{
	int i, sz;
	unsigned int pending;
	unsigned char *pptr = pixels;
	uint32_t offs = 0;

	wait_vsync();

	/* assume window is always at 0 at the beginning */
	pending = fb_size;
	while(pending > 0) {
		sz = pending > vmode->bank_size ? vmode->bank_size : pending;
		memcpy(vmem, pptr, sz);
		pptr += sz;
		pending -= sz;
		vbe_setwin(0, ++offs);
	}

	vbe_setwin(0, 0);
}
