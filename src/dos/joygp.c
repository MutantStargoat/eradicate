#include <conio.h>
#include <dos.h>
#include "joy.h"

#define MAX_COUNT	2048
#define MIN_COUNT	32

static unsigned int read_joy(int *xret, int *yret);

int have_joy;
unsigned int joy_bnstate, joy_bndiff, joy_bnpress;
int16_t joy_pos[2];

int cal_min[2], cal_cent[2], cal_max[2];
int rawcnt[2];

int joy_detect(void)
{
	/*
	int xcnt, ycnt;

	read_joy(&xcnt, &ycnt);
	return have_joy = (xcnt < MAX_COUNT || ycnt < MAX_COUNT);
	*/
	return 0;	/* TODO disable until we add a calibration screen */
}

void joy_update(void)
{
	int i, rng;
	unsigned int prev_bnstate;

	prev_bnstate = joy_bnstate;
	joy_bnstate = read_joy(rawcnt, rawcnt + 1);

	for(i=0; i<2; i++) {
		if(rawcnt[i] < MAX_COUNT) {
			if(cal_cent[i] == 0) cal_cent[i] = rawcnt[i];
			if(cal_min[i] == 0 || rawcnt[i] < cal_min[i]) cal_min[i] = rawcnt[i];
			if(rawcnt[i] > cal_max[i]) cal_max[i] = rawcnt[i];

			if(rawcnt[i] < cal_cent[i]) {
				if((rng = cal_cent[i] - cal_min[i])) {
					joy_pos[i] = ((rawcnt[i] - cal_min[i]) << 15) / rng - 0x8000;
				} else {
					joy_pos[i] = 0;
				}
			} else {
				if((rng = cal_max[i] - cal_cent[i] + 1)) {
					joy_pos[i] = ((rawcnt[i] - cal_cent[i]) << 15) / rng;
				} else {
					joy_pos[i] = 0;
				}
			}
		} else {
			joy_pos[i] = 0;
		}

		/* if the range is too short, discard any movement and center */
		rng = cal_max[i] - cal_min[i];
		if(rng < MIN_COUNT) {
			joy_pos[i] = 0;
		}

		if(joy_pos[i] <= -0x4000) {
			joy_bnstate |= JOY_LEFT << (i << 1);
		} else if(joy_pos[i] > 0x4000) {
			joy_bnstate |= JOY_RIGHT << (i << 1);
		}
	}

	joy_bndiff = joy_bnstate ^ prev_bnstate;
	joy_bnpress = joy_bnstate & joy_bndiff;
}

#define JOY_PORT	0x201

#ifdef __DJGPP__
#define outp(p, v)	outportb(p, v)
#define inp(p)		inportb(p)
#endif

static unsigned int read_joy(int *xret, int *yret)
{
	int i, pending = 2;
	unsigned char val = 0xff, diff, prev;

	*xret = *yret = MAX_COUNT;

	_disable();

	outp(JOY_PORT, 0xff);
	for(i=0; i<MAX_COUNT; i++) {
		prev = val;
		val = inp(JOY_PORT);
		diff = val ^ prev;

		if(diff & 1) {
			*xret = i;
			if(--pending <= 0) break;
		}
		if(diff & 2) {
			*yret = i;
			if(--pending <= 0) break;
		}
	}

	_enable();

	return ~val & 0xf0;
}
