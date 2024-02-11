#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "keyb.h"
#include "timer.h"
#include "vidsys.h"
#include "logger.h"
#include "cdpmi.h"
#include "joy.h"
#include "input.h"
#include "audio.h"
#include "cpuid.h"

static int quit;

int force_snd_config;


int main(int argc, char **argv)
{
	int i;
	int vmidx, status = 0;
	char *env;

	for(i=1; i<argc; i++) {
		if(strcmp(argv[i], "-setup") == 0) {
			force_snd_config = 1;
		}
	}

#ifdef __DJGPP__
	__djgpp_nearptr_enable();
#endif

	if((env = getenv("GAMELOG"))) {
		init_logger(env);
	} else {
		init_logger("game.log");
	}

#ifdef __WATCOMC__
	printf("watcom build\n");
#elif defined(__DJGPP__)
	printf("djgpp build\n");
#endif

	if(read_cpuid(&cpuid) == 0) {
		print_cpuid(&cpuid);
	}

	/* au_init needs to be called early, before init_timer, and also before
	 * we enter graphics mode, to use the midas configuration tool if necessary
	 */
	if(au_init() == -1) {
		return 1;
	}

	init_timer(100);
	kb_init();

	if(vid_init() == -1) {
		return 1;
	}

	if((vmidx = vid_findmode(640, 480, 16)) == -1) {
		return 1;
	}
	if(!(vmem = vid_setmode(vmidx))) {
		return 1;
	}

	if(init(argc, argv) == -1) {
		status = -1;
		goto break_evloop;
	}

	fflush(stdout);
	reset_timer();

	for(;;) {
		int key;
		while((key = kb_getkey()) != -1) {
			game_key(key, 1);
			if(quit) goto break_evloop;
		}

		inp_update();

		time_msec = get_msec();
		draw();
	}

break_evloop:
	cleanup();
	vid_cleanup();
	kb_shutdown();
	au_shutdown();
	return status;
}

void game_quit(void)
{
	quit = 1;
}

void demo_abort(void)
{
	vid_cleanup();
	stop_logger();
	print_tail("game.log");
	exit(1);
}
