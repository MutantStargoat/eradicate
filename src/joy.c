#include "joy.h"
#include "game.h"

/* platform-independent joystick helper routines */

/* call this to generate fake keystrokes from joystick input */
void joy_keyemu(void)
{
	if(!have_joy || !key_event) {
		return;	/* no joy ... */
	}

	if(joy_bnpress & JOY_UP) {
		key_event(KB_UP, 1);
	}
	if(joy_bnpress & JOY_DOWN) {
		key_event(KB_DOWN, 1);
	}
	if(joy_bnpress & JOY_LEFT) {
		key_event(KB_LEFT, 1);
	}
	if(joy_bnpress & JOY_RIGHT) {
		key_event(KB_RIGHT, 1);
	}
	if(joy_bnpress & JOY_BN0) {
		key_event('\n', 1);
	}
	if(joy_bnpress & JOY_BN1) {
		key_event(27, 1);
	}
}
