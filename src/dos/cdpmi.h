#ifndef CDPMI_H_
#define CDPMI_H_

#include <stdlib.h>
#include "inttypes.h"

#pragma pack (push, 1)
struct dpmi_regs {
	uint32_t edi, esi, ebp;
	uint32_t reserved;
	uint32_t ebx, edx, ecx, eax;
	uint16_t flags;
	uint16_t es, ds, fs, gs;
	uint16_t ip, cs, sp, ss;
};
#pragma pack (pop)

enum {
	FLAGS_CF	= 0x000001,
	FLAGS_PF	= 0x000004,
	FLAGS_ZF	= 0x000040,
	FLAGS_SF	= 0x000080,
	FLAGS_IF	= 0x000020,
	FLAGS_DF	= 0x000040,
	FLAGS_VM	= 0x020000,
	FLAGS_ID	= 0x200000,
};

int dpmi_init(void);
void dpmi_cleanup(void);
void *dpmi_lowbuf(void);

uint16_t dpmi_alloc(unsigned int par, uint16_t *sel);
void dpmi_free(uint16_t sel);
int dpmi_rmint(int inum, struct dpmi_regs *regs);
void *dpmi_mmap(uint32_t phys_addr, unsigned int size);
void dpmi_munmap(void *addr);

#endif	/* CDPMI_H_ */
