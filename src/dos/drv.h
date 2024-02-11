#ifndef DRV_H_
#define DRV_H_

#include "inttypes.h"

struct vid_drvops {
	int (*init)(void);
	void (*cleanup)(void);

	int (*setmode)(int mode);
	int (*curmode)(void);
};

#define MAX_DRV	16
extern struct vid_driver *vid_drvlist[MAX_DRV];
extern int vid_numdrv;

extern void *vid_vmem;
extern int vid_vmem_size;

void vid_register_vga(void);		/* drv_vga.c */
void vid_register_vbe(void);		/* drv_vbe.c */

#endif	/* DRV_H_ */
