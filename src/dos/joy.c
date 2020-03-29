#include "joy.h"

unsigned int joy_bnstate, joy_bndiff, joy_bnpress;
int16_t joy_pos[2];

static unsigned int joy_bnprev;

int joy_detect(void)
{
	return 0;
}

void joy_update(void)
{
}
