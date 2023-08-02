#ifndef VGA_H_
#define VGA_H_

#include "inttypes.h"
#include "dosutil.h"
#include "cdpmi.h"
#include "vgaregs.h"

#define VGA_FBADDR	((void*)phys_to_virt(0xa0000))

int vga_setmode(int mode);
int vga_setmodex(void);

#define vga_planemask(mask)		vga_sc_write(VGA_SC_MAPMASK_REG, mask)

#ifdef __WATCOMC__
void vga_setpal(int16_t idx, uint8_t r, uint8_t g, uint8_t b);
#pragma aux vga_setpal = \
	"test ax, 0x8000" \
	"jnz skip_dacaddr" \
	"mov dx, 0x3c8" \
	"out dx, al" \
	"skip_dacaddr:" \
	"mov dx, 0x3c9" \
	"mov al, bl" \
	"shr al, 2" \
	"out dx, al" \
	"mov al, bh" \
	"shr al, 2" \
	"out dx, al" \
	"mov al, cl" \
	"shr al, 2" \
	"out dx, al" \
	parm[ax][bl][bh][cl] \
	modify[dx];
#endif /* __WATCOMC__ */

#define vga_sc_write(reg, data) \
	outpw(VGA_SC_ADDR_PORT, (uint16_t)(reg) | ((uint16_t)(data) << 8))
#define vga_sc_read(reg) \
	(outp(VGA_SC_ADDR_PORT, reg), inp(VGA_SC_DATA_PORT))
#define vga_crtc_write(reg, data) \
	outpw(VGA_CRTC_PORT, (uint16_t)(reg) | ((uint16_t)(data) << 8))
#define vga_crtc_read(reg) \
	(outp(VGA_CRTC_ADDR_PORT, reg), inp(VGA_CRTC_DATA_PORT))
#define vga_crtc_wrmask(reg, data, mask) \
	outp(VGA_CRTC_DATA_PORT, (crtc_read(reg) & ~(mask)) | (data))

#endif	/* VGA_H_ */
