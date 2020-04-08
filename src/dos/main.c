#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "keyb.h"
#include "timer.h"
#include "gfx.h"
#include "logger.h"
#include "cdpmi.h"
#include "joy.h"
#include "input.h"
#include "audio.h"

static int quit;

int main(int argc, char **argv)
{
	int vmidx, status = 0;

#ifdef __DJGPP__
	__djgpp_nearptr_enable();
#endif

	init_logger("game.log");

	/* au_init needs to be called early, before init_timer, and also before
	 * we enter graphics mode, to use the midas configuration tool if necessary
	 */
	if(au_init() == -1) {
		return 1;
	}

	init_timer(100);
	kb_init(32);

	if(init_video() == -1) {
		return 1;
	}

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

		inp_update();

		time_msec = get_msec();
		draw();
	}

break_evloop:
	cleanup();
	set_text_mode();
	cleanup_video();
	kb_shutdown();
	au_shutdown();
	return status;
}

void game_quit(void)
{
	quit = 1;
}
