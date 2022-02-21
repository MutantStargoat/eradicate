#include <stdio.h>
#include <stdlib.h>
#include <ddraw.h>
#include "gfx.h"
#include "options.h"

#define SAME_BPP(a, b)	\
	((a) == (b) || ((a) == 16 && (b) == 15) || ((a) == 15 && (b) == 16) || \
	 ((a) == 32 && (b) == 24) || ((a) == 24 && (b) == 32))


void msgbox(const char *msg);	/* defined in ddraw/main.c */

static HRESULT WINAPI enum_mode(DDSURFACEDESC *sdesc, void *cls);
static int countbits(unsigned int x);
static int firstbit(unsigned int x);
static int vmcmp(const void *a, const void *b);


extern HWND win;

static struct video_mode *vmodes;
static int num_vmodes, max_vmodes;
static struct video_mode *cur_vmode;
static int pgsize;

static IDirectDraw *ddraw;
static IDirectDrawSurface *ddfront, *ddback;
static IDirectDrawPalette *ddpalette;
static RECT win_frame_offs;


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

	return num_vmodes ? 0 : -1;
}

static void reset_ddraw(void)
{
	if(!ddraw) return;

	if(ddpalette) {
		IDirectDrawPalette_Release(ddpalette);
		ddpalette = 0;
	}
	if(ddfront) {
		IDirectDrawSurface_Release(ddfront);
		ddfront = 0;
	}
	IDirectDraw_RestoreDisplayMode(ddraw);
	IDirectDraw_SetCooperativeLevel(ddraw, win, DDSCL_NORMAL);
}

void cleanup_video(void)
{
	free(vmodes);
	if(ddraw) {
		reset_ddraw();
		IDirectDraw_Release(ddraw);
		ddraw = 0;
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
	int i, res, best = -1;
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
		if(bpp == 16 && (res = match_video_mode(xsz, ysz, 32)) != -1) {
			return res;
		}
		fprintf(stderr, "failed to find video mode %dx%d %d bpp)\n", xsz, ysz, bpp);
		return -1;
	}
	return best;
}


void *set_video_mode(int idx, int nbuf)
{
	DDSURFACEDESC sd;
	DDSCAPS caps;
	struct video_mode *vm = vmodes + idx;
	RECT winrect;

	if(cur_vmode == vm) return (void*)0xbadf00d;

	printf("setting video mode %dx%d %d bpp\n", vm->xsz, vm->ysz, vm->bpp);

	if(opt.fullscreen) {
		SetWindowLong(win, GWL_STYLE, WS_POPUP);
		SetWindowPos(win, HWND_TOPMOST, 0, 0, vm->xsz, vm->ysz, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

		if(IDirectDraw_SetCooperativeLevel(ddraw, win, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) != 0) {
			fprintf(stderr, "failed to grab exclusive video access, will run with potentially reduced performance\n");
		}

		if(IDirectDraw_SetDisplayMode(ddraw, vm->xsz, vm->ysz, vm->bpp) != 0) {
			fprintf(stderr, "failed to set video mode %dx%d %d bpp\n", vm->xsz, vm->ysz, vm->bpp);
			return 0;
		}
	} else {
		int width, height;

		IDirectDraw_SetCooperativeLevel(ddraw, win, DDSCL_NORMAL);

		SetWindowLong(win, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(win, HWND_TOP, 10, 10, vm->xsz, vm->ysz, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

		GetWindowRect(win, &winrect);
		winrect.right = winrect.left + vm->xsz;
		winrect.bottom = winrect.top + vm->ysz;
		win_frame_offs = winrect;

		AdjustWindowRect(&winrect, WS_OVERLAPPEDWINDOW, 0);

		win_frame_offs.left -= winrect.left;
		win_frame_offs.right = winrect.right - win_frame_offs.right;
		win_frame_offs.top -= winrect.top;
		win_frame_offs.bottom = winrect.bottom - win_frame_offs.bottom;
		printf("window frame offsets: X %d/%d  Y %d/%d\n", win_frame_offs.left, win_frame_offs.right,
				win_frame_offs.top, win_frame_offs.bottom);

		width = winrect.right - winrect.left;
		height = winrect.bottom - winrect.top;

		if(winrect.left < 0) winrect.left = 0;
		if(winrect.top < 0) winrect.top = 0;

		MoveWindow(win, winrect.left, winrect.top, width, height, 0);
	}

	if(vm->bpp <= 8) {
		unsigned int palflags;
		static PALETTEENTRY palinit[256];

		switch(vm->bpp) {
		case 1:
			palflags = DDPCAPS_1BIT;
			break;
		case 2:
			palflags = DDPCAPS_2BIT;
			break;
		case 4:
			palflags = DDPCAPS_4BIT;
			break;
		default:
			palflags = DDPCAPS_8BIT | DDPCAPS_ALLOW256;
		}
		if(IDirectDraw_CreatePalette(ddraw, palflags, palinit, &ddpalette, 0) != 0) {
			fprintf(stderr, "failed to create palette\n");
			return 0;
		}
	}

	memset(&sd, 0, sizeof sd);
	sd.dwSize = sizeof sd;
	if(opt.fullscreen) {
		sd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		sd.dwBackBufferCount = 1;
	} else {
		sd.dwFlags = DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	if(ddfront) {
		IDirectDrawSurface_Release(ddfront);
	}
	if(IDirectDraw_CreateSurface(ddraw, &sd, &ddfront, 0) != 0) {
		fprintf(stderr, "failed to create primary DirectDraw surface\n");
		return 0;
	}
	if(vm->bpp <= 8) {
		IDirectDrawSurface_SetPalette(ddfront, ddpalette);
	}

	if(opt.fullscreen) {
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		if(IDirectDrawSurface_GetAttachedSurface(ddfront, &caps, &ddback) != 0) {
			fprintf(stderr, "failed to get back buffer surface\n");
			return 0;
		}
	} else {
		memset(&sd, 0, sizeof sd);
		sd.dwSize = sizeof sd;
		sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		sd.dwWidth = vm->xsz;
		sd.dwHeight = vm->ysz;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

		if(ddback) {
			IDirectDrawSurface_Release(ddback);
		}
		if(IDirectDraw_CreateSurface(ddraw, &sd, &ddback, 0) != 0) {
			fprintf(stderr, "failed to create back buffer surface\n");
			return 0;
		}
	}

	cur_vmode = vm;
	pgsize = vm->ysz * vm->pitch;

	if(resizefb(vm->xsz, vm->ysz, 16) == -1) {
		fprintf(stderr, "failed to allocate %dx%d (%d bpp) framebuffer\n", vm->xsz,
				vm->ysz, vm->bpp);
		return 0;
	}

	return (void*)0xbadf00d;
}

int set_text_mode(void)
{
	reset_ddraw();
	cur_vmode = 0;
	return 0;
}

void wait_vsync(void)
{
	IDirectDraw_WaitForVerticalBlank(ddraw, DDWAITVB_BLOCKBEGIN, 0);
}

void blit_frame(void *pixels, int vsync)
{
	int i, j, res;
	DDSURFACEDESC sd = {0};
	uint16_t *pptr;
	uint32_t *dest32;

	sd.dwSize = sizeof sd;

	if((res = IDirectDrawSurface_Lock(ddback, 0, &sd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK |
			DDLOCK_WRITEONLY, 0)) != 0) {
		printf("lock failed: ", res);
		switch(res) {
		case DDERR_INVALIDOBJECT:
			printf("DDERR_INVALIDOBJECT\n");
			break;
		case DDERR_INVALIDPARAMS:
			printf("DDERR_INVALIDPARAMS\n");
			break;
		case DDERR_OUTOFMEMORY:
			printf("DDERR_OUTOFMEMORY\n");
			break;
		case DDERR_SURFACEBUSY:
			printf("DDERR_SURFACEBUSY\n");
			break;
		case DDERR_SURFACELOST:
			printf("DDERR_SURFACELOST\n");
			break;
		case DDERR_WASSTILLDRAWING:
			printf("DDERR_WASSTILLDRAWING\n");
			break;
		default:
			printf("unknown\n");
		}
		return;
	}

	switch(cur_vmode->bpp) {
	case 15:
	case 16:
		memcpy(sd.lpSurface, pixels, sd.dwHeight * sd.lPitch);
		break;

	case 32:
		pptr = pixels;
		dest32 = sd.lpSurface;
		for(i=0; i<cur_vmode->ysz; i++) {
			for(j=0; j<cur_vmode->xsz; j++) {
				dest32[j] = ((((*pptr >> 8) & 0xf8) << 16) |
						(((*pptr >> 3) & 0xfc) << 8) |
						((*pptr << 3) & 0xf8));
				*pptr++;
			}
			dest32 = (uint32_t*)((char*)dest32 + sd.lPitch);
		}
		break;

	default:
		break;
	}


	IDirectDrawSurface_Unlock(ddback, 0);

	if(opt.fullscreen) {
		if((res = IDirectDrawSurface_Flip(ddfront, 0, DDFLIP_WAIT)) != 0) {
			if(res == DDERR_SURFACELOST && IDirectDrawSurface_Restore(ddfront) == 0) {
				IDirectDrawSurface_Flip(ddfront, 0, DDFLIP_WAIT);
			}
		}
	} else {
		RECT winrect;
		GetWindowRect(win, &winrect);
		winrect.left += win_frame_offs.left;
		winrect.right -= win_frame_offs.right;
		winrect.top += win_frame_offs.top;
		winrect.bottom -= win_frame_offs.bottom;
		IDirectDrawSurface_Blt(ddfront, &winrect, ddback, 0, DDBLT_WAIT, 0);
	}
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
