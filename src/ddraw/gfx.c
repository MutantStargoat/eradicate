#include <stdio.h>
#include <stdlib.h>
#include <ddraw.h>
#include "gfx.h"

#define SAME_BPP(a, b)	\
	((a) == (b) || ((a) == 16 && (b) == 15) || ((a) == 15 && (b) == 16) || \
	 ((a) == 32 && (b) == 24) || ((a) == 24 && (b) == 32))


void msgbox(const char *msg);	/* defined in ddraw/main.c */

static HRESULT WINAPI enum_mode(DDSURFACEDESC *sdesc, void *cls);
static int countbits(unsigned int x);
static int firstbit(unsigned int x);
static int vmcmp(const void *a, const void *b);


static struct video_mode *vmodes;
static int num_vmodes, max_vmodes;
static struct video_mode *cur_vmode;

static IDirectDraw *ddraw;
static IDirectDrawSurface *ddsurf;


int init_video(void)
{
	int i;
	struct video_mode *vm;

	if(DirectDrawCreate(0, &ddraw, 0) != 0) {
		msgbox("failed to initialize DirectDraw");
		return -1;
	}

	IDirectDraw_EnumDisplayModes(ddraw, 0, 0, 0, enum_mode);
	qsort(vmodes, num_vmodes, sizeof *vmodes, vmcmp);

	vm = vmodes;
	printf("Available video modes:\n");
	for(i=0; i<num_vmodes; i++) {
		printf(" - %dx%d %d bpp", vm->xsz, vm->ysz, vm->bpp);
		if(vm->rbits | vm->gbits | vm->bbits) {
			printf("  %d%d%d  (%x %x %x)", vm->rbits, vm->gbits, vm->bbits,
					vm->rmask, vm->gmask, vm->bmask);
		}
		putchar('\n');
		vm++;
	}

	return -1;
}

void cleanup_video(void)
{
	free(vmodes);
	if(ddraw) {
		IDirectDraw_Release(ddraw);
	}
}


struct video_mode *video_modes(void)
{
	return vmodes;
}

int num_video_modes(void)
{
	return num_vmodes;
}

struct video_mode *get_video_mode(int idx)
{
	if(idx == VMODE_CURRENT) {
		return cur_vmode;
	}
	return vmodes + idx;
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


void *set_video_mode(int idx, int nbuf)
{
	return 0;
}

void wait_vsync(void)
{
}

void blit_frame(void *pixels, int vsync)
{
}

#define RGB_OR_INDEXED	(DDPF_RGB | DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 | \
		DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXEDTO8)

static HRESULT WINAPI enum_mode(DDSURFACEDESC *sdesc, void *cls)
{
	struct video_mode *vm;
	DDPIXELFORMAT *pixfmt = &sdesc->ddpfPixelFormat;

	if(!(pixfmt->dwFlags & RGB_OR_INDEXED)) {
		return DDENUMRET_OK;	/* skip any mode which is not RGB or palettized */
	}

	if(num_vmodes >= max_vmodes) {
		void *tmp;
		int newsz = max_vmodes ? max_vmodes << 1 : 8;
		if(!(tmp = realloc(vmodes, newsz * sizeof *vmodes))) {
			msgbox("failed to grow video modes array");
			abort();
		}
		vmodes = tmp;
		max_vmodes = newsz;
	}

	vm = vmodes + num_vmodes++;
	memset(vm, 0, sizeof *vm);
	vm->xsz = sdesc->dwWidth;
	vm->ysz = sdesc->dwHeight;
	vm->bpp = pixfmt->dwRGBBitCount;
	vm->pitch = sdesc->lPitch;
	if(pixfmt->dwFlags & DDPF_RGB) {
		vm->rbits = countbits(pixfmt->dwRBitMask);
		vm->gbits = countbits(pixfmt->dwGBitMask);
		vm->bbits = countbits(pixfmt->dwBBitMask);
		vm->rshift = firstbit(pixfmt->dwRBitMask);
		vm->gshift = firstbit(pixfmt->dwGBitMask);
		vm->bshift = firstbit(pixfmt->dwBBitMask);
		vm->rmask = pixfmt->dwRBitMask;
		vm->gmask = pixfmt->dwGBitMask;
		vm->bmask = pixfmt->dwBBitMask;
	}
	vm->max_pages = sdesc->dwBackBufferCount + 1;

	return DDENUMRET_OK;
}


static int countbits(unsigned int x)
{
	int i, count = 0;
	for(i=0; i<32; i++) {
		count += x & 1;
		x >>= 1;
	}
	return count;
}

static int firstbit(unsigned int x)
{
	int i;
	for(i=0; i<32; i++) {
		if(x & 1) break;
		x >>= 1;
	}
	return i;
}

static int vmcmp(const void *a, const void *b)
{
	const struct video_mode *va = a;
	const struct video_mode *vb = b;

	if(va->xsz != vb->xsz) return va->xsz - vb->xsz;
	if(va->ysz != vb->ysz) return va->ysz - vb->ysz;
	if(va->bpp != vb->bpp) return va->bpp - vb->bpp;
	return va - vb;
}
