#ifndef VBE_H_
#define VBE_H_

#include <stdio.h>
#include "inttypes.h"
#include "util.h"

#pragma pack (push, 1)
struct vbe_info {
	char sig[4];
	uint16_t ver;
	char *oem_name;
	uint32_t caps;
	uint16_t *modes;
	uint16_t vmem_blk;	/* video memory size in 64k blocks */
	uint16_t oem_ver;
	char *vendor;
	char *product;
	char *revstr;
	uint16_t accel_ver;
	uint16_t *accel_modes;
	char reserved[216];
	char oem_data[256];
} PACKED;

struct vbe_mode_info {
	uint16_t attr;
	uint8_t wina_attr, winb_attr;
	uint16_t win_gran, win_size;
	uint16_t wina_seg, winb_seg;
	uint32_t win_func;
	uint16_t scanline_bytes;

	/* VBE 1.2 and above */
	uint16_t xres, yres;
	uint8_t xcharsz, ycharsz;
	uint8_t num_planes;
	uint8_t bpp;
	uint8_t num_banks;
	uint8_t mem_model;
	uint8_t bank_size;		/* bank size in KB */
	uint8_t num_img_pages;
	uint8_t reserved1;

	/* direct color fields */
	uint8_t rsize, rpos;
	uint8_t gsize, gpos;
	uint8_t bsize, bpos;
	uint8_t xsize, xpos;
	uint8_t cmode_info;		/* direct color mode attributes */

	/* VBE 2.0 and above */
	uint32_t fb_addr;		/* physical address of the linear framebuffer */
	uint32_t os_addr;		/* phys. address of off-screen memory */
	uint16_t os_size;		/* size in KB of off-screen memory */

	/* VBE 3.0 and above */
	uint16_t lfb_scanline_bytes;
	uint8_t banked_num_img_pages;
	uint8_t lfb_num_img_pages;
	uint8_t lfb_rsize, lfb_rpos;
	uint8_t lfb_gsize, lfb_gpos;
	uint8_t lfb_bsize, lfb_bpos;
	uint8_t lfb_xsize, lfb_xpos;
	uint32_t max_pixel_clock;

	char reserved2[190];
} PACKED;

struct vbe_crtc_info {
	uint16_t htotal, hsync_start, hsync_end;
	uint16_t vtotal, vsync_start, vsync_end;
	uint8_t flags;
	uint32_t pixel_clock;
	uint16_t rate_centihz;	/* refresh rate in 1/100 hz (pck / (htotal * vtotal)) */
	char reserved[40];
} PACKED;
#pragma pack (pop)

/* returned by vbe_scanline_info() */
struct vbe_scanline_info {
	int size;
	int num_pixels;
	int max_scanlines;
};

enum {
	VBE_8BIT_DAC	= 0x01,
	VBE_NON_VGA		= 0x02,
	VBE_DAC_BLANK	= 0x04,
	VBE_STEREO		= 0x08,	/* ? */
	VBE_ACCEL		= 0x08,
	VBE_STEREO_VESA	= 0x10,	/* ? */
	VBE_MUSTLOCK	= 0x10,
	VBE_HWCURSOR	= 0x20,
	VBE_HWCLIP		= 0x40,
	VBE_TRANSP_BLT	= 0x80
};

#define VBE_VER_MAJOR(v)	(((v) >> 8) & 0xff)
#define VBE_VER_MINOR(v)	((v) & 0xff)

/* VBE mode attribute flags (vbe_mode_info.attr) */
enum {
	VBE_ATTR_AVAIL		= 0x0001,
	VBE_ATTR_OPTINFO	= 0x0002,
	VBE_ATTR_TTY		= 0x0004,
	VBE_ATTR_COLOR		= 0x0008,
	VBE_ATTR_GFX		= 0x0010,
	/* VBE 2.0 */
	VBE_ATTR_NOTVGA		= 0x0020,
	VBE_ATTR_BANKED		= 0x0040,
	VBE_ATTR_LFB		= 0x0080,
	VBE_ATTR_DBLSCAN	= 0x0100,
	/* VBE 3.0 */
	VBE_ATTR_ILACE		= 0x0200,	/* ! */
	VBE_ATTR_TRIPLEBUF	= 0x0400,
	VBE_ATTR_STEREO		= 0x0800,
	VBE_ATTR_STEREO_2FB	= 0x1000,
	/* VBE/AF */
	VBE_ATTR_MUSTLOCK	= 0x0200	/* ! */
};

/* VBE memory model type (vbe_mode_info.mem_model) */
enum {
	VBE_TYPE_TEXT,
	VBE_TYPE_CGA,
	VBE_TYPE_HERCULES,
	VBE_TYPE_PLANAR,
	VBE_TYPE_PACKED,
	VBE_TYPE_UNCHAIN,
	VBE_TYPE_DIRECT,
	VBE_TYPE_YUV
};

/* VBE window attribute (vbe_mode_info.win(a|b)_attr) */
enum {
	VBE_WIN_AVAIL	= 0x01,
	VBE_WIN_RD		= 0x02,
	VBE_WIN_WR		= 0x04
};

/* mode number flags */
enum {
	VBE_MODE_RATE		= 0x0800,	/* VBE 3.0+ user-specified refresh rate */
	VBE_MODE_ACCEL		= 0x2000,	/* VBE/AF */
	VBE_MODE_LFB		= 0x4000,	/* VBE 2.0+ */
	VBE_MODE_PRESERVE	= 0x8000
};

/* standard mode numbers */
enum {
	VBE_640X400_8BPP	= 0x100,
	VBE_640X480_8BPP	= 0x101,
	VBE_800X600_4BPP	= 0x102,
	VBE_800X600_8BPP	= 0x103,
	VBE_1024X768_4BPP	= 0x104,
	VBE_1024X768_8BPP	= 0x105,
	VBE_1280X1024_4BPP	= 0x106,
	VBE_1280X1024_8BPP	= 0x107,
	VBE_80X60_TEXT		= 0x108,
	VBE_132X25_TEXT		= 0x109,
	VBE_132X43_TEXT		= 0x10a,
	VBE_132X50_TEXT		= 0x10b,
	VBE_132X60_TEXT		= 0x10c,
	/* VBE 1.2 */
	VBE_320X200_15BPP	= 0x10d,
	VBE_320X200_16BPP	= 0x10e,
	VBE_320X200_24BPP	= 0x10f,
	VBE_640X480_15BPP	= 0x110,
	VBE_640X480_16BPP	= 0x111,
	VBE_640X480_24BPP	= 0x112,
	VBE_800X600_15BPP	= 0x113,
	VBE_800X600_16BPP	= 0x114,
	VBE_800X600_24BPP	= 0x115,
	VBE_1024X768_15BPP	= 0x116,
	VBE_1024X768_16BPP	= 0x117,
	VBE_1024X768_24BPP	= 0x118,
	VBE_1280X1024_15BPP	= 0x119,
	VBE_1280X1024_16BPP	= 0x11a,
	VBE_1280X1024_24BPP	= 0x11b,
	/* VBE 2.0 */
	VBE_1600X1200_8BPP	= 0x120,
	VBE_1600X1200_15BPP	= 0x121,
	VBE_1600X1200_16BPP	= 0x122,

	VBE_VMEM_MODE		= 0x81ff
};

/* VBE CRTC flags (vbe_crtc_info.flags) */
enum {
	VBE_CRTC_DBLSCAN	= 0x01,
	VBE_CRTC_ILACE		= 0x02,
	VBE_CRTC_HSYNC_NEG	= 0x04,
	VBE_CRTC_VSYNC_NEG	= 0x08
};

enum {
	VBE_STATE_CTRLHW	= 0x01,
	VBE_STATE_BIOS		= 0x02,
	VBE_STATE_DAC		= 0x04,
	VBE_STATE_REGS		= 0x08,

	VBE_STATE_ALL		= 0xffff
};

enum {
	VBE_SWAP_NOW,
	VBE_SWAP_VBLANK,
	VBE_SWAP_ASYNC	/* schedule swap and return (triple-buffering) */
};

int vbe_info(struct vbe_info *info);
int vbe_num_modes(struct vbe_info *info);
int vbe_mode_info(int mode, struct vbe_mode_info *minf);

void vbe_print_info(FILE *fp, struct vbe_info *info);
void vbe_print_mode_info(FILE *fp, struct vbe_mode_info *minf);

int vbe_setmode(uint16_t mode);
int vbe_setmode_crtc(uint16_t mode, struct vbe_crtc_info *crtc);
int vbe_getmode(void);

int vbe_state_size(unsigned int flags);
int vbe_save(void *stbuf, int sz, unsigned int flags);
int vbe_restore(void *stbuf, int sz, unsigned int flags);

int vbe_setwin(int wid, int pos);
int vbe_getwin(int wid);

/* returns the actual length in pixels, which might not be what was requested */
int vbe_setscanlen(int len_pix);
int vbe_getscanlen(void);
int vbe_getpitch(void);
int vbe_scanline_info(struct vbe_scanline_info *sinf);

int vbe_setdisp(int x, int y, int when);
int vbe_swap(uint32_t voffs, int when);
int vbe_swap_pending(void);	/* 0: not pending (done) or error, 1: pending swap */
/* TODO add stereo swap */

#endif	/* VBE_H_ */
