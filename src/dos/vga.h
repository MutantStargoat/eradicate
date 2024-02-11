#ifndef VGA_H_
#define VGA_H_

/* ---- VGA registers ---- */
#define VGA_AC_PORT		0x3c0
#define VGA_AC_RD_PORT		0x3c1
#define VGA_SC_ADDR_PORT	0x3c4
#define VGA_SC_DATA_PORT	0x3c5
#define VGA_DAC_STATUS_PORT	0x3c7
#define VGA_DAC_RADDR_PORT	0x3c7
#define VGA_DAC_WADDR_PORT	0x3c8
#define VGA_DAC_DATA_PORT	0x3c9
#define VGA_GC_ADDR_PORT	0x3ce
#define VGA_GC_DATA_PORT	0x3cf
#define VGA_CRTC_PORT		0x3d4
#define VGA_CRTC_ADDR_PORT	0x3d4
#define VGA_CRTC_DATA_PORT	0x3d5
#define VGA_STAT0_PORT		0x3c2
#define VGA_STAT1_PORT		0x3da
#define VGA_MISC_PORT		0x3c2
#define VGA_MISC_RD_PORT	0x3cc

/* attribute controller registers */
#define VGA_AC_EN		0x20
#define VGA_AC_MODE_REG		0x10

/* sequence controller registers */
#define VGA_SC_RESET_REG	0x00
#define VGA_SC_CLOCK_REG	0x01
#define VGA_SC_MAPMASK_REG	0x02
#define VGA_SC_MEMMODE_REG	0x04

/* graphics controller registers */
#define VGA_GC_SR_REG		0x00
#define VGA_GC_SREN_REG		0x01
#define VGA_GC_ROT_REG		0x03
#define VGA_GC_MODE_REG		0x05
#define VGA_GC_MASK_REG		0x08

/* attribute controller mode register (10h) bits */
#define VGA_AC_MODE_GFX		0x01
#define VGA_AC_MODE_MONO	0x02
#define VGA_AC_MODE_LGE		0x04
#define VGA_AC_MODE_BLINK	0x08
#define VGA_AC_MODE_PIXPAN	0x20
#define VGA_AC_MODE_8BIT	0x40

/* misc register bits */
#define VGA_MISC_COLOR		0x01
#define VGA_MISC_CPUEN		0x02
#define VGA_MISC_CLK25		0
#define VGA_MISC_CLK28		0x04
#define VGA_MISC_PG1		0x20
#define VGA_MISC_400		0
#define VGA_MISC_350		0x40
#define VGA_MISC_480		0xc0


/* CRTC registers */
#define CRTC_HTOTAL_REG		0x00
#define CRTC_HEND_REG		0x01
#define CRTC_HBLSTART_REG	0x02
#define CRTC_HBLEND_REG		0x03
#define CRTC_HRETSTART_REG	0x04
#define CRTC_HRETEND_REG	0x05
#define CRTC_VTOTAL_REG		0x06
#define CRTC_OVF_REG		0x07
#define CRTC_PRESET_REG		0x08
#define CRTC_MAXSCAN_REG	0x09
#define CRTC_CURSTART_REG	0x0a
#define CRTC_CUREND_REG		0x0b
#define CRTC_STARTH_REG		0x0c
#define CRTC_STARTL_REG		0x0d
#define CRTC_CURH_REG		0x0e
#define CRTC_CURL_REG		0x0f
#define CRTC_VRETSTART_REG	0x10
#define CRTC_VRETEND_REG	0x11
#define CRTC_VEND_REG		0x12
#define CRTC_OFFSET_REG		0x13
#define CRTC_UL_REG		0x14
#define CRTC_VBLSTART_REG	0x15
#define CRTC_VBLEND_REG		0x16
#define CRTC_MODE_REG		0x17
#define CRTC_LCMP_REG		0x18

/* CRTC register bits */
#define CRTC_VRETEND_PR		0x80

void vga_vsync(void);
void vga_setpal(int idx, int count, const struct vid_color *col);
void vga_getpal(int idx, int count, struct vid_color *col);

#endif	/* VGA_H_ */
