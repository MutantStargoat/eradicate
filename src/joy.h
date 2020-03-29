#ifndef JOY_H_
#define JOY_H_

#include "inttypes.h"

enum {
	JOY_LEFT	= 0x01,
	JOY_RIGHT	= 0x02,
	JOY_UP		= 0x04,
	JOY_DOWN	= 0x08,
	JOY_BN0		= 0x10,
	JOY_BN1		= 0x20,
	JOY_BN2		= 0x40,
	JOY_BN3		= 0x80
};

#define JOY_BN_ANY	0xfff0

extern int have_joy;
extern unsigned int joy_bnstate, joy_bndiff, joy_bnpress;
extern int16_t joy_pos[2];

int joy_detect(void);
void joy_update(void);
void joy_keyemu(void);

#endif	/* JOY_H_ */
