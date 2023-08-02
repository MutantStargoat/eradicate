#ifndef DPMI_H_
#define DPMI_H_

#ifdef __DJGPP__
#include <dpmi.h>
#include <sys/nearptr.h>

#define virt_to_phys(v)	((v) + __djgpp_base_address)
#define phys_to_virt(p)	((p) - __djgpp_base_address)

#else	/* not djgpp (basically watcom) */

#define virt_to_phys(v)	(v)
#define phys_to_virt(p)	(p)

#endif	/* __DJGPP__ */

#include "inttypes.h"
#include "util.h"

#pragma pack (push, 1)
struct dpmi_regs {
	uint32_t edi, esi, ebp;
	uint32_t reserved;
	uint32_t ebx, edx, ecx, eax;
	uint16_t flags;
	uint16_t es, ds, fs, gs;
	uint16_t ip, cs, sp, ss;
} PACKED;
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

uint16_t dpmi_alloc(unsigned int par, uint16_t *sel);
void dpmi_free(uint16_t sel);
void dpmi_int(int inum, struct dpmi_regs *regs);
void *dpmi_mmap(uint32_t phys_addr, unsigned int size);
void dpmi_munmap(void *addr);

#ifdef __WATCOMC__
#pragma aux dpmi_alloc = \
		"mov ax, 0x100" \
		"int 0x31" \
		"mov [edi], dx" \
		"jnc alloc_skip_err" \
		"xor ax, ax" \
		"alloc_skip_err:" \
		value[ax] \
		parm[ebx][edi] \
		modify[dx];

#pragma aux dpmi_free = \
		"mov ax, 0x101" \
		"int 0x31" \
		parm[dx] \
		modify[ax];

#pragma aux dpmi_int = \
		"mov ax, 0x300" \
		"xor ecx, ecx" \
		"int 0x31" \
		parm[ebx][edi] \
		modify[ax ecx];

#pragma aux dpmi_mmap = \
		"mov ax, 0x800" \
		"mov cx, bx" \
		"shr ebx, 16" \
		"mov di, si" \
		"shr esi, 16" \
		"int 0x31" \
		"jnc mmap_skip_err" \
		"xor bx, bx" \
		"xor cx, cx" \
	"mmap_skip_err:" \
		"mov ax, bx" \
		"shl eax, 16" \
		"mov ax, cx" \
		value[eax] \
		parm[ebx][esi] \
		modify[bx cx di esi];

#pragma aux dpmi_munmap = \
		"mov ax, 0x801" \
		"mov cx, bx" \
		"shr ebx, 16" \
		"int 0x31" \
		parm[ebx] \
		modify[ax cx ebx];
#endif	/* __WATCOMC__ */

#ifdef __DJGPP__
#define dpmi_int(inum, regs) __dpmi_int((inum), (__dpmi_regs*)(regs))
#endif

#endif	/* DPMI_H_ */
