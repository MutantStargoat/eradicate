#include <stdio.h>
#include <i86.h>
#include "cdpmi.h"

#define LOWBUF_SIZE		8192
#define RMSTACK_SIZE	4096

static char *lowbuf, *lowfree;
static uint16_t lowbuf_sel, lowbuf_seg;

int dpmi_init(void)
{
	if(!(lowbuf_seg = dpmi_alloc(LOWBUF_SIZE >> 4, &lowbuf_sel))) {
		fprintf(stderr, "DPMI init failed to allocate low memory buffer\n");
		return -1;
	}
	lowbuf = (char*)((intptr_t)lowbuf_seg << 4);
	lowfree = lowbuf + RMSTACK_SIZE;
	return 0;
}

void dpmi_cleanup(void)
{
	if(!lowbuf_sel) return;
	dpmi_free(lowbuf_sel);
	lowbuf = 0;
	lowbuf_sel = 0;
	lowbuf_seg = 0;
}

void *dpmi_lowbuf(void)
{
	return lowfree;
}

uint16_t dpmi_alloc(unsigned int par, uint16_t *sel)
{
	union REGS regs = {0};

	regs.w.ax = 0x100;
	regs.w.bx = par;
	int386(0x31, &regs, &regs);
	if(regs.w.cflag != 0) {
		return 0;
	}
	*sel = regs.w.dx;
	return regs.w.ax;
}

void dpmi_free(uint16_t sel)
{
	union REGS regs = {0};

	regs.w.ax = 0x101;
	regs.w.dx = sel;
	int386(0x31, &regs, &regs);
}

int dpmi_rmint(int inum, struct dpmi_regs *dregs)
{
	union REGS regs = {0};
	struct SREGS sregs = {0};

	regs.x.eax = 0x300;
	regs.x.ebx = inum;
	sregs.es = FP_SEG(dregs);
	regs.x.edi = FP_OFF(dregs);
	sregs.ss = lowbuf_seg;	/* 4k real mode stack */
	int386x(0x31, &regs, &regs, &sregs);
	if(regs.x.cflag != 0) {
		return -1;
	}
	return 0;
}

void *dpmi_mmap(uint32_t phys_addr, unsigned int size)
{
	union REGS regs = {0};

	regs.w.ax = 0x800;
	regs.w.bx = phys_addr >> 16;
	regs.w.cx = phys_addr & 0xffff;
	regs.w.si = size >> 16;
	regs.w.di = size & 0xffff;
	int386(0x31, &regs, &regs);
	if(regs.w.cflag != 0) {
		return 0;
	}

	return (void*)(((intptr_t)regs.w.bx << 16) | (intptr_t)regs.w.cx);
}

void dpmi_munmap(void *ptr)
{
	union REGS regs = {0};
	intptr_t addr = (intptr_t)ptr;

	regs.w.ax = 0x801;
	regs.w.bx = addr >> 16;
	regs.w.cx = addr & 0xffff;
	int386(0x31, &regs, &regs);
}
