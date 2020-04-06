#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include "vbe.h"
#include "cdpmi.h"


#define FIXPTR(ptr) \
	do { \
		uint32_t paddr = (uint32_t)(ptr); \
		uint16_t pseg = paddr >> 16; \
		uint16_t poffs = paddr & 0xffff; \
		if(pseg == seg && poffs < 512) { \
			paddr = ((uint32_t)seg << 4) + poffs; \
		} else { \
			paddr = ((uint32_t)pseg << 4) + poffs; \
		} \
		(ptr) = (void*)phys_to_virt(paddr); \
	} while(0)

/* hijack the "VESA" sig field, to pre-cache number of modes */
#define NMODES(inf) *(uint16_t*)((inf)->sig)
#define NACCMODES(inf) *(uint16_t*)((inf)->sig + 2)

static int cur_pitch;
/* TODO update cur_pitch on mode-change and on setscanlen */


int vbe_info(struct vbe_info *info)
{
	void *lowbuf;
	uint16_t seg, sel;
	uint16_t *modeptr;
	uint32_t offs;
	struct dpmi_regs regs = {0};

	assert(sizeof *info == 512);

	if(!(seg = dpmi_alloc(sizeof *info / 16, &sel))) {
		return -1;
	}
	lowbuf = (void*)phys_to_virt((uint32_t)seg << 4);

	memcpy(lowbuf, "VBE2", 4);

	regs.eax = 0x4f00;
	regs.es = seg;
	regs.edi = 0;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		fprintf(stderr, "vbe_get_info (4f00) failed\n");
		dpmi_free(sel);
		return -1;
	}

	memcpy(info, lowbuf, sizeof *info);
	dpmi_free(sel);

	FIXPTR(info->oem_name);
	FIXPTR(info->vendor);
	FIXPTR(info->product);
	FIXPTR(info->revstr);
	FIXPTR(info->modes);
	/* implementations without the accel capability, will use the space taken
	 * by the accel_modes pointer for other data (probably video modes). We
	 * need to check for the capability before "fixing" this pointer, otherwise
	 * we'll shuffle random data.
	 */
	if(info->caps & VBE_ACCEL) {
		FIXPTR(info->accel_modes);
	}

	/* info->modes should be pointing somewhere at the end of the original
	 * low memory buffer. make it point at the same offset in the info
	 * buffer where we copied everything instead.
	 */
	offs = (char*)info->modes - (char*)lowbuf;
	if(offs < sizeof *info) {	/* this should always be true */
		info->modes = (uint16_t*)((char*)info + offs);
	}

	modeptr = info->modes;
	while(*modeptr != 0xffff) {
		if(modeptr - info->modes >= 256) {
			modeptr = info->modes;
			break;
		}
		modeptr++;
	}
	NMODES(info) = modeptr - info->modes;

	if(info->caps & VBE_ACCEL) {
		modeptr = info->accel_modes;
		while(*modeptr != 0xffff) {
			if(modeptr - info->accel_modes >= 256) {
				modeptr = info->accel_modes;
				break;
			}
			modeptr++;
		}
		NACCMODES(info) = modeptr - info->accel_modes;
	}
	return 0;
}

int vbe_num_modes(struct vbe_info *info)
{
	return NMODES(info);
}

int vbe_mode_info(int mode, struct vbe_mode_info *minf)
{
	void *lowbuf;
	uint16_t seg, sel;
	struct dpmi_regs regs = {0};

	assert(sizeof *minf == 256);
	assert(offsetof(struct vbe_mode_info, max_pixel_clock) == 0x3e);

	if(!(seg = dpmi_alloc(sizeof *minf / 16, &sel))) {
		return -1;
	}
	lowbuf = (void*)phys_to_virt((uint32_t)seg << 4);

	regs.eax = 0x4f01;
	regs.ecx = mode;
	regs.es = seg;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		fprintf(stderr, "vbe_mode_info (4f01) failed\n");
		dpmi_free(sel);
		return -1;
	}

	memcpy(minf, lowbuf, sizeof *minf);
	dpmi_free(sel);
	return 0;
}

void vbe_print_info(FILE *fp, struct vbe_info *vinf)
{
	fprintf(fp, "vbe version: %u.%u\n", VBE_VER_MAJOR(vinf->ver), VBE_VER_MINOR(vinf->ver));
	if(VBE_VER_MAJOR(vinf->ver) >= 2) {
		fprintf(fp, "%s - %s (%s)\n", vinf->vendor, vinf->product, vinf->revstr);
		if(vinf->caps & VBE_ACCEL) {
			fprintf(fp, "vbe/af %d.%d\n", VBE_VER_MAJOR(vinf->accel_ver), VBE_VER_MINOR(vinf->accel_ver));
		}
	} else {
		fprintf(fp, "oem: %s\n", vinf->oem_name);
	}
	fprintf(fp, "video memory: %dkb\n", vinf->vmem_blk * 64);

	if(vinf->caps) {
		fprintf(fp, "caps:");
		if(vinf->caps & VBE_8BIT_DAC) fprintf(fp, " dac8");
		if(vinf->caps & VBE_NON_VGA) fprintf(fp, " non-vga");
		if(vinf->caps & VBE_DAC_BLANK) fprintf(fp, " dac-blank");
		if(vinf->caps & VBE_ACCEL) fprintf(fp, " af");
		if(vinf->caps & VBE_MUSTLOCK) fprintf(fp, " af-lock");
		if(vinf->caps & VBE_HWCURSOR) fprintf(fp, " af-curs");
		if(vinf->caps & VBE_HWCLIP) fprintf(fp, " af-clip");
		if(vinf->caps & VBE_TRANSP_BLT) fprintf(fp, " af-tblt");
		fprintf(fp, "\n");
	}

	fprintf(fp, "%d video modes available\n", NMODES(vinf));
	if(vinf->caps & VBE_ACCEL) {
		fprintf(fp, "%d accelerated (VBE/AF) modes available\n", NACCMODES(vinf));
	}
	fflush(fp);
}

void vbe_print_mode_info(FILE *fp, struct vbe_mode_info *minf)
{
	fprintf(fp, "%dx%d %dbpp", minf->xres, minf->yres, minf->bpp);

	switch(minf->mem_model) {
	case VBE_TYPE_DIRECT:
		fprintf(fp, " (rgb");
		if(0) {
	case VBE_TYPE_YUV:
			fprintf(fp, " (yuv");
		}
		fprintf(fp, " %d%d%d)", minf->rsize, minf->gsize, minf->bsize);
		break;
	case VBE_TYPE_PLANAR:
		fprintf(fp, " (%d planes)", minf->num_planes);
		break;
	case VBE_TYPE_PACKED:
		fprintf(fp, " (packed)");
		break;
	case VBE_TYPE_TEXT:
		fprintf(fp, " (%dx%d cells)", minf->xcharsz, minf->ycharsz);
		break;
	case VBE_TYPE_CGA:
		fprintf(fp, " (CGA)");
		break;
	case VBE_TYPE_UNCHAIN:
		fprintf(fp, " (unchained-%d)", minf->num_planes);
		break;
	}
	fprintf(fp, " %dpg", minf->num_img_pages);

	if(minf->attr & VBE_ATTR_LFB) {
		fprintf(fp, " lfb@%lx", (unsigned long)minf->fb_addr);
	} else {
		fprintf(fp, " %xkb/bank", (unsigned int)minf->bank_size);
	}

	fprintf(fp, " [");
	if(minf->attr & VBE_ATTR_AVAIL) fprintf(fp, " avail");
	if(minf->attr & VBE_ATTR_OPTINFO) fprintf(fp, " opt");
	if(minf->attr & VBE_ATTR_TTY) fprintf(fp, " tty");
	if(minf->attr & VBE_ATTR_COLOR) fprintf(fp, " color");
	if(minf->attr & VBE_ATTR_GFX) fprintf(fp, " gfx");
	if(minf->attr & VBE_ATTR_NOTVGA) fprintf(fp, " non-vga");
	if(minf->attr & VBE_ATTR_BANKED) fprintf(fp, " banked");
	if(minf->attr & VBE_ATTR_LFB) fprintf(fp, " lfb");
	if(minf->attr & VBE_ATTR_DBLSCAN) fprintf(fp, " dblscan");
	if(minf->attr & VBE_ATTR_ILACE) fprintf(fp, " ilace");
	if(minf->attr & VBE_ATTR_TRIPLEBUF) fprintf(fp, " trplbuf");
	if(minf->attr & VBE_ATTR_STEREO) fprintf(fp, " stereo");
	if(minf->attr & VBE_ATTR_STEREO_2FB) fprintf(fp, " stdual");
	fprintf(fp, " ]\n");
	fflush(fp);
}

int vbe_setmode(uint16_t mode)
{
	struct dpmi_regs regs = {0};

	regs.eax = 0x4f02;
	regs.ebx = mode;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}

	cur_pitch = vbe_getpitch();
	return 0;
}

int vbe_setmode_crtc(uint16_t mode, struct vbe_crtc_info *crtc)
{
	void *lowbuf;
	uint16_t seg, sel;
	struct dpmi_regs regs = {0};

	assert(sizeof *crtc == 59);

	if(!(seg = dpmi_alloc((sizeof *crtc + 15) / 16, &sel))) {
		return -1;
	}
	lowbuf = (void*)phys_to_virt((uint32_t)seg << 4);

	memcpy(lowbuf, crtc, sizeof *crtc);

	regs.eax = 0x4f02;
	regs.ebx = mode;
	regs.es = seg;
	dpmi_int(0x10, &regs);

	dpmi_free(sel);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}

	cur_pitch = vbe_getpitch();
	return 0;
}

int vbe_getmode(void)
{
	struct dpmi_regs regs = {0};

	regs.eax = 0x4f03;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return regs.ebx & 0xffff;
}

int vbe_state_size(unsigned int flags)
{
	struct dpmi_regs regs = {0};

	regs.eax = 0x4f04;
	regs.edx = 0;
	regs.ecx = flags;
	dpmi_int(0x10, &regs);
	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return (regs.ebx & 0xffff) * 64;
}

int vbe_save(void *stbuf, int sz, unsigned int flags)
{
	void *lowbuf;
	uint16_t seg, sel;
	struct dpmi_regs regs = {0};

	if(!(seg = dpmi_alloc((sz + 15) / 16, &sel))) {
		return -1;
	}
	lowbuf = (void*)phys_to_virt((uint32_t)seg << 4);

	regs.eax = 0x4f04;
	regs.edx = 1;	/* save */
	regs.ecx = flags;
	regs.es = seg;
	dpmi_int(0x10, &regs);
	if((regs.eax & 0xffff) != 0x4f) {
		dpmi_free(sel);
		return -1;
	}

	memcpy(stbuf, lowbuf, sz);
	dpmi_free(sel);
	return 0;
}

int vbe_restore(void *stbuf, int sz, unsigned int flags)
{
	void *lowbuf;
	uint16_t seg, sel;
	struct dpmi_regs regs = {0};

	if(!(seg = dpmi_alloc((sz + 15) / 16, &sel))) {
		return -1;
	}
	lowbuf = (void*)phys_to_virt((uint32_t)seg << 4);

	memcpy(lowbuf, stbuf, sz);

	regs.eax = 0x4f04;
	regs.edx = 2;	/* restore */
	regs.ecx = flags;
	regs.es = seg;
	dpmi_int(0x10, &regs);
	dpmi_free(sel);
	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return 0;
}

int vbe_setwin(int wid, int pos)
{
	struct dpmi_regs regs;

	if(wid & ~1) return -1;

	regs.eax = 0x4f05;
	regs.ebx = wid;
	regs.edx = pos;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return 0;
}

int vbe_getwin(int wid)
{
	struct dpmi_regs regs;

	if(wid & ~1) return -1;

	regs.eax = 0x4f05;
	regs.ebx = wid;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}

	return regs.edx & 0xffff;
}

int vbe_setscanlen(int len_pix)
{
	struct dpmi_regs regs;

	regs.eax = 0x4f06;
	regs.ebx = 0;	/* set scanline length in pixels */
	regs.ecx = len_pix;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}

	cur_pitch = vbe_getpitch();
	return regs.ecx;
}

int vbe_getscanlen(void)
{
	struct dpmi_regs regs;

	regs.eax = 0x4f06;
	regs.ebx = 1;	/* get scanline length */
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return regs.ecx;
}

int vbe_getpitch(void)
{
	struct dpmi_regs regs;

	regs.eax = 0x4f06;
	regs.ebx = 1;	/* get scanline length */
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return regs.ebx;
}

int vbe_scanline_info(struct vbe_scanline_info *sinf)
{
	struct dpmi_regs regs;

	regs.eax = 0x4f06;
	regs.ebx = 1;	/* get scanline length */
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}

	sinf->size = regs.ebx & 0xffff;
	sinf->num_pixels = regs.ecx & 0xffff;
	sinf->max_scanlines = regs.edx & 0xffff;
	return 0;
}

enum {
	SDISP_SET			= 0x00,
	SDISP_GET			= 0x01,
	SDISP_ALTSET		= 0x02,
	SDISP_SET_STEREO	= 0x03,
	SDISP_GETSCHED		= 0x04,
	SDISP_STEREO_ON		= 0x05,
	SDISP_STEREO_OFF	= 0x06,
	SDISP_SET_VBLANK	= 0x80,
	SDISP_ALTSET_VBLANK	= 0x82,
	SDISP_SET_STEREO_VBLANK	= 0x83
};

int vbe_setdisp(int x, int y, int when)
{
	struct dpmi_regs regs;

	regs.eax = 0x4f07;
	regs.ebx = (when == VBE_SWAP_VBLANK) ? SDISP_SET_VBLANK : SDISP_SET;
	regs.ecx = x;
	regs.edx = y;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return 0;
}

int vbe_swap(uint32_t voffs, int when)
{
	struct dpmi_regs regs;
	int op;

	switch(when) {
	case VBE_SWAP_ASYNC:
		op = SDISP_ALTSET;
		break;

	case VBE_SWAP_NOW:
		/* XXX is this the only way? */
		return vbe_setdisp(voffs % cur_pitch, voffs / cur_pitch, when);

	case VBE_SWAP_VBLANK:
	default:
		op = SDISP_ALTSET_VBLANK;
		break;
	}


	regs.eax = 0x4f07;
	regs.ebx = op;
	regs.ecx = voffs;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return -1;
	}
	return 0;
}

int vbe_swap_pending(void)
{
	struct dpmi_regs regs;

	regs.eax = 0x4f07;
	regs.ebx = SDISP_GETSCHED;
	dpmi_int(0x10, &regs);

	if((regs.eax & 0xffff) != 0x4f) {
		return 0;
	}
	return regs.ecx;
}
