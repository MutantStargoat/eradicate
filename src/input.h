#ifndef INPUT_H_
#define INPUT_H_

/* keyboard and joystick mappings */
enum {
	INP_FWD,
	INP_BRK,
	INP_LTURN,
	INP_RTURN,
	INP_LBRK,
	INP_RBRK,
	INP_FIRE,
	INP_CAM,
	NUM_INPUTS
};

#define INP_FWD_BIT		(1 << INP_FWD)
#define INP_BRK_BIT		(1 << INP_BRK)
#define INP_LTURN_BIT	(1 << INP_LTURN)
#define INP_RTURN_BIT	(1 << INP_RTURN)
#define INP_LBRK_BIT	(1 << INP_LBRK)
#define INP_RBRK_BIT	(1 << INP_RBRK)
#define INP_FIRE_BIT	(1 << INP_FIRE)
#define INP_CAM_BIT		(1 << INP_CAM)

extern int keymap[NUM_INPUTS][2];
extern int joymap[NUM_INPUTS];

extern unsigned int inpstate, inppress;

void inp_update(void);

#endif	/* INPUT_H_ */
