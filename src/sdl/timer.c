#include <SDL/SDL.h>
#include "timer.h"

static unsigned long start_time;

void init_timer(int res_hz)
{
}

void reset_timer(void)
{
	start_time = SDL_GetTicks();
}

unsigned long get_msec(void)
{
	return SDL_GetTicks() - start_time;
}

void sleep_msec(unsigned long msec)
{
	SDL_Delay(msec);
}
