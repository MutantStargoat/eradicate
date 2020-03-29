#include "input.h"
#include "game.h"
#include "joy.h"

int keymap[NUM_INPUTS][2] = {
	{'w', KB_UP},
	{'s', KB_DOWN},
	{'a', KB_LEFT},
	{'d', KB_RIGHT},
	{'q', 'z'},
	{'e', 'x'},
	{' ', KB_NUM_0},
	{'\t', -1}
};

int joymap[NUM_INPUTS] = {
	JOY_BN0,
	JOY_DOWN,
	JOY_LEFT,
	JOY_RIGHT,
	-1,
	-1,
	JOY_BN1,
	-1
};

unsigned int inpstate, inppress;

void inp_update(void)
{
	int i;
	unsigned int bit = 1;
	static unsigned int inpprev, inpdiff;

	inpprev = inpstate;
	inpstate = 0;

	for(i=0; i<NUM_INPUTS; i++) {
		if(kb_isdown(keymap[i][0]) || kb_isdown(keymap[i][1])) {
			inpstate |= bit;
		}
		bit <<= 1;
	}

	if(have_joy) {
		joy_update();
		bit = 1;
		for(i=0; i<NUM_INPUTS; i++) {
			if(joy_bnstate & joymap[i]) {
				inpstate |= bit;
			}
			bit <<= 1;
		}
	}

	inpdiff = inpstate ^ inpprev;
	inppress = inpstate & inpdiff;

	if(input_event) {
		bit = 1;
		for(i=0; i<NUM_INPUTS; i++) {
			if(inppress & bit) {
				input_event(i);
			}
			bit <<= 1;
		}
	}
}
