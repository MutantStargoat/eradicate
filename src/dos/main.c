#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "keyb.h"
#include "timer.h"
#include "gfx.h"
#include "logger.h"
#include "cdpmi.h"

static int quit;

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

	if(init(argc, argv) == -1) {
		status = -1;
		goto break_evloop;
	}

	reset_timer();

	for(;;) {
		int key;
		if(key_event) {
			while((key = kb_getkey()) != -1) {
				key_event(key, 1);
			}
		} else {
			while((key = kb_getkey()) != -1) {
				if(key == 27) goto break_evloop;
			}
		}
		if(quit) goto break_evloop;

		time_msec = get_msec();
		draw();
	}

break_evloop:
	free(fb_buf);
	cleanup();
	set_text_mode();
	cleanup_video();
	kb_shutdown();
	return status;
}

void game_quit(void)
{
	quit = 1;
}
