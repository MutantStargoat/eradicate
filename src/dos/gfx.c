#include <stdio.h>
#include <stdlib.h>
#include "gfx.h"
#include "vbe.h"
#include "vga.h"

#define SAME_BPP(a, b)	\
	((a) == (b) || ((a) == 16 && (b) == 15) || ((a) == 15 && (b) == 16) || \
	 ((a) == 32 && (b) == 24) || ((a) == 24 && (b) == 32))

static uint32_t calc_mask(int sz, int pos);

static struct video_mode *vmodes;
static int num_vmodes;

static int vbe_init_ver;
static struct vbe_info vbe;

/* current mode */
static struct video_mode *curmode;
static void *vpgaddr[2];
static int fbidx;
static int pgcount, pgsize, fbsize;

int init_video(void)
{
	int i, num, max_modes;
	struct video_mode *vmptr;

	if(vbe_info(&vbe) == -1) {
		fprintf(stderr, "failed to retrieve VBE information\n");
		return -1;
	}
	vbe_print_info(stdout, &vbe);

	num_vmodes = 0;
	max_modes = 4;	/* TODO change */
	if(!(vmodes = malloc(max_modes * sizeof *vmodes))) {
		fprintf(stderr, "failed to allocate video modes list\n");
		return -1;
	}

	num = vbe_num_modes(&vbe);
	for(i=0; i<num; i++) {
		struct vbe_mode_info minf;

		if(vbe_mode_info(vbe.modes[i], &minf) == -1) {
			continue;
		}

		if(num_vmodes >= max_modes) {
			int newmax = max_modes ? (max_modes << 1) : 16;
			if(!(vmptr = realloc(vmodes, newmax * sizeof *vmodes))) {
				fprintf(stderr, "failed to grow video mode list (%d)\n", newmax);
				free(vmodes);
				return -1;
			}
			vmodes = vmptr;
			max_modes = newmax;
		}

		vmptr = vmodes + num_vmodes++;
		memset(vmptr, 0, sizeof *vmptr);
		vmptr->mode = vbe.modes[i];
		vmptr->xsz = minf.xres;
		vmptr->ysz = minf.yres;
		vmptr->bpp = minf.bpp;
		vmptr->pitch = minf.scanline_bytes;
		if(minf.mem_model == VBE_TYPE_DIRECT) {
			vmptr->rbits = minf.rsize;
			vmptr->gbits = minf.gsize;
			vmptr->bbits = minf.bsize;
			vmptr->rshift = minf.rpos;
			vmptr->gshift = minf.gpos;
			vmptr->bshift = minf.bpos;
			vmptr->rmask = calc_mask(minf.rsize, minf.rpos);
			vmptr->gmask = calc_mask(minf.gsize, minf.gpos);
			vmptr->bmask = calc_mask(minf.bsize, minf.bpos);
		}
		vmptr->fb_addr = minf.fb_addr;
		vmptr->max_pages = minf.num_img_pages;

		printf("%04x: ", vbe.modes[i]);
		vbe_print_mode_info(stdout, &minf);
	}
	fflush(stdout);

	vbe_init_ver = VBE_VER_MAJOR(vbe.ver);
	if(vbe_init_ver < 2) {
		fprintf(stderr, "VBE 2.0 required. Upgrade your video card or run univbe.\n");
		free(vmodes);
		vmodes = 0;
		return -1;
	}
	return 0;
}

void cleanup_video(void)
{
	free(vmodes);
}

struct video_mode *video_modes(void)
{
	return vmodes;
}

int num_video_modes(void)
{
	return num_vmodes;
}

int match_video_mode(int xsz, int ysz, int bpp)
{
	int i, best = -1;
	struct video_mode *vm;

	for(i=0; i<num_vmodes; i++) {
		vm = vmodes + i;
		if(vm->xsz != xsz || vm->ysz != ysz) continue;
		if(SAME_BPP(vm->bpp, bpp)) {
			best = i;
		}
		if(vm->bpp == bpp) break;
	}

	if(best == -1) {
		fprintf(stderr, "failed to find video mode %dx%d %d bpp)\n", xsz, ysz, bpp);
		return -1;
	}
	return best;
}

int find_video_mode(int mode)
{
	int i;
	struct video_mode *vm;

	vm = vmodes;
	for(i=0; i<num_vmodes; i++) {
		if(vm->mode == mode) return i;
	}
	return -1;
}

void *set_video_mode(int idx, int nbuf)
{
	unsigned int mode;
	struct video_mode *vm = vmodes + idx;

	printf("setting video mode %x (%dx%d %d bpp)\n", (unsigned int)vm->mode,
			vm->xsz, vm->ysz, vm->bpp);
	fflush(stdout);

	mode = vm->mode | VBE_MODE_LFB;
	if(vbe_setmode(mode) == -1) {
		mode = vm->mode;
		if(vbe_setmode(mode) == -1) {
			fprintf(stderr, "failed to set video mode %x\n", (unsigned int)vm->mode);
			return 0;
		}
		printf("Warning: failed to get a linear framebuffer. falling back to banked mode\n");
	}

	curmode = vm;
	if(nbuf < 1) nbuf = 1;
	if(nbuf > 2) nbuf = 2;
	pgcount = nbuf > vm->max_pages ? vm->max_pages : nbuf;
	pgsize = (vm->xsz * vm->bpp / 8) * vm->ysz;
	fbsize = pgcount * pgsize;

	printf("pgcount: %d, pgsize: %d, fbsize: %d\n", pgcount, pgsize, fbsize);
	printf("phys addr: %p\n", (void*)vm->fb_addr);

	vpgaddr[0] = (void*)dpmi_mmap(vm->fb_addr, fbsize);
	if(!vpgaddr[0]) {
		fprintf(stderr, "failed to map framebuffer (phys: %lx, size: %d)\n",
				(unsigned long)vm->fb_addr, fbsize);
		set_text_mode();
		return 0;
	}
	//memset(vpgaddr[0], 0xaa, fbsize);

	printf("vaddr: %p\n", vpgaddr[0]);
	fflush(stdout);

	if(pgcount > 1) {
		vpgaddr[1] = (char*)vpgaddr[0] + pgsize;
		fbidx = 1;
		page_flip(FLIP_NOW);	/* start with the second page visible */
	} else {
		fbidx = 0;
		vpgaddr[1] = 0;
	}
	return vpgaddr[0];
}

int set_text_mode(void)
{
	vga_setmode(3);
	curmode = 0;
	return 0;
}

void *page_flip(int vsync)
{
	if(!vpgaddr[1]) {
		/* page flipping not supported */
		return vpgaddr[0];
	}

	vbe_swap(fbidx ? pgsize : 0, vsync ? VBE_SWAP_VBLANK : VBE_SWAP_NOW);
	fbidx = (fbidx + 1) & 1;

	return vpgaddr[fbidx];
}

static uint32_t calc_mask(int sz, int pos)
{
	int i;
	uint32_t mask = 0;
	while(sz-- > 0) {
		mask = (mask << 1) | 1;
	}
	return mask << pos;
}
