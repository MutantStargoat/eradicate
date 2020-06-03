#include <string.h>
#include "game.h"
#include "gfxutil.h"

enum {
	IN		= 0,
	LEFT	= 1,
	RIGHT	= 2,
	TOP		= 4,
	BOTTOM	= 8
};

static int outcode(int x, int y, int xmin, int ymin, int xmax, int ymax)
{
	int code = 0;

	if(x < xmin) {
		code |= LEFT;
	} else if(x > xmax) {
		code |= RIGHT;
	}
	if(y < ymin) {
		code |= TOP;
	} else if(y > ymax) {
		code |= BOTTOM;
	}
	return code;
}

#define FIXMUL(a, b)	(((a) * (b)) >> 8)
#define FIXDIV(a, b)	(((a) << 8) / (b))

#define LERP(a, b, t)	((a) + FIXMUL((b) - (a), (t)))

int clip_line(int *x0, int *y0, int *x1, int *y1, int xmin, int ymin, int xmax, int ymax)
{
	int oc_out;

	int oc0 = outcode(*x0, *y0, xmin, ymin, xmax, ymax);
	int oc1 = outcode(*x1, *y1, xmin, ymin, xmax, ymax);

	long fx0, fy0, fx1, fy1, fxmin, fymin, fxmax, fymax;

	if(!(oc0 | oc1)) return 1;	/* both points are inside */

	fx0 = *x0 << 8;
	fy0 = *y0 << 8;
	fx1 = *x1 << 8;
	fy1 = *y1 << 8;
	fxmin = xmin << 8;
	fymin = ymin << 8;
	fxmax = xmax << 8;
	fymax = ymax << 8;

	for(;;) {
		long x, y, t;

		if(oc0 & oc1) return 0;		/* both have points with the same outbit, not visible */
		if(!(oc0 | oc1)) break;		/* both points are inside */

		oc_out = oc0 ? oc0 : oc1;

		if(oc_out & TOP) {
			t = FIXDIV(fymin - fy0, fy1 - fy0);
			x = LERP(fx0, fx1, t);
			y = fymin;
		} else if(oc_out & BOTTOM) {
			t = FIXDIV(fymax - fy0, fy1 - fy0);
			x = LERP(fx0, fx1, t);
			y = fymax;
		} else if(oc_out & LEFT) {
			t = FIXDIV(fxmin - fx0, fx1 - fx0);
			x = fxmin;
			y = LERP(fy0, fy1, t);
		} else if(oc_out & RIGHT) {
			t = FIXDIV(fxmax - fx0, fx1 - fx0);
			x = fxmax;
			y = LERP(fy0, fy1, t);
		}

		if(oc_out == oc0) {
			fx0 = x;
			fy0 = y;
			oc0 = outcode(fx0 >> 8, fy0 >> 8, xmin, ymin, xmax, ymax);
		} else {
			fx1 = x;
			fy1 = y;
			oc1 = outcode(fx1 >> 8, fy1 >> 8, xmin, ymin, xmax, ymax);
		}
	}

	*x0 = fx0 >> 8;
	*y0 = fy0 >> 8;
	*x1 = fx1 >> 8;
	*y1 = fy1 >> 8;
	return 1;
}

void draw_line(int x0, int y0, int x1, int y1, unsigned short color)
{
	int i, dx, dy, x_inc, y_inc, error;
	unsigned short *fb = fb_pixels;

	fb += y0 * fb_width + x0;

	dx = x1 - x0;
	dy = y1 - y0;

	if(dx >= 0) {
		x_inc = 1;
	} else {
		x_inc = -1;
		dx = -dx;
	}
	if(dy >= 0) {
		y_inc = fb_width;
	} else {
		y_inc = -fb_width;
		dy = -dy;
	}

	if(dx > dy) {
		error = dy * 2 - dx;
		for(i=0; i<=dx; i++) {
			*fb = color;
			if(error >= 0) {
				error -= dx * 2;
				fb += y_inc;
			}
			error += dy * 2;
			fb += x_inc;
		}
	} else {
		error = dx * 2 - dy;
		for(i=0; i<=dy; i++) {
			*fb = color;
			if(error >= 0) {
				error -= dy * 2;
				fb += x_inc;
			}
			error += dx * 2;
			fb += y_inc;
		}
	}
}


#define BLUR(w, h, pstep, sstep) \
	for(i=0; i<h; i++) { \
		int r, g, b; \
		int rsum = UNPACK_R16(sptr[0]) * (rad + 1); \
		int gsum = UNPACK_G16(sptr[0]) * (rad + 1); \
		int bsum = UNPACK_B16(sptr[0]) * (rad + 1); \
		int count = (rad * 2 + 1) << 8; \
		int midsize = w - rad * 2; \
		int rfirstpix = UNPACK_R16(sptr[0]); \
		int rlastpix = UNPACK_R16(sptr[pstep * (w - 1)]); \
		int gfirstpix = UNPACK_G16(sptr[0]); \
		int glastpix = UNPACK_G16(sptr[pstep * (w - 1)]); \
		int bfirstpix = UNPACK_B16(sptr[0]); \
		int blastpix = UNPACK_B16(sptr[pstep * (w - 1)]); \
		/* add up the contributions for the -1 pixel */ \
		for(j=0; j<rad; j++) { \
			rsum += UNPACK_R16(sptr[pstep * j]); \
			gsum += UNPACK_G16(sptr[pstep * j]); \
			bsum += UNPACK_B16(sptr[pstep * j]); \
		} \
		/* first part adding sptr[rad] and subtracting sptr[0] */ \
		for(j=0; j<=rad; j++) { \
			rsum += UNPACK_R16((int)sptr[pstep * rad]) - rfirstpix; \
			gsum += UNPACK_G16((int)sptr[pstep * rad]) - gfirstpix; \
			bsum += UNPACK_B16((int)sptr[pstep * rad]) - bfirstpix; \
			sptr += pstep; \
			r = scale * rsum / count; \
			g = scale * gsum / count; \
			b = scale * bsum / count; \
			*dptr = PACK_RGB16(r, g, b); \
			dptr += pstep; \
		} \
		/* middle part adding sptr[rad] and subtracting sptr[-(rad+1)] */ \
		for(j=1; j<midsize; j++) { \
			rsum += UNPACK_R16((int)sptr[pstep * rad]) - UNPACK_R16((int)sptr[-(rad + 1) * pstep]); \
			gsum += UNPACK_G16((int)sptr[pstep * rad]) - UNPACK_G16((int)sptr[-(rad + 1) * pstep]); \
			bsum += UNPACK_B16((int)sptr[pstep * rad]) - UNPACK_B16((int)sptr[-(rad + 1) * pstep]); \
			sptr += pstep; \
			r = scale * rsum / count; \
			g = scale * gsum / count; \
			b = scale * bsum / count; \
			*dptr = PACK_RGB16(r, g, b); \
			dptr += pstep; \
		} \
		/* last part adding lastpix and subtracting sptr[-(rad+1)] */ \
		for(j=0; j<rad; j++) { \
			rsum += rlastpix - UNPACK_R16((int)sptr[-(rad + 1) * pstep]); \
			gsum += glastpix - UNPACK_G16((int)sptr[-(rad + 1) * pstep]); \
			bsum += blastpix - UNPACK_B16((int)sptr[-(rad + 1) * pstep]); \
			sptr += pstep; \
			r = scale * rsum / count; \
			g = scale * gsum / count; \
			b = scale * bsum / count; \
			*dptr = PACK_RGB16(r, g, b); \
			dptr += pstep; \
		} \
		sptr += sstep; \
		dptr += sstep; \
	}

/* TODO bound blur rad to image size to avoid inner loop conditionals */
/* TODO make version with pow2 (rad*2+1) to avoid div with count everywhere */
void blur_horiz(uint16_t *dest, uint16_t *src, int xsz, int ysz, int rad, int scale)
{
	int i, j;
	uint16_t *dptr = dest;
	uint16_t *sptr = src;

	BLUR(xsz, ysz, 1, 0);
}


void blur_vert(uint16_t *dest, uint16_t *src, int xsz, int ysz, int rad, int scale)
{
	int i, j;
	uint16_t *dptr = dest;
	uint16_t *sptr = src;
	int pixel_step = xsz;
	int scanline_step = 1 - ysz * pixel_step;

	BLUR(ysz, xsz, pixel_step, scanline_step);
}

void convimg_rgb24_rgb16(uint16_t *dest, unsigned char *src, int xsz, int ysz)
{
	int i;
	int npixels = xsz * ysz;

	for(i=0; i<npixels; i++) {
		int r = *src++;
		int g = *src++;
		int b = *src++;
		*dest++ = PACK_RGB16(r, g, b);
	}
}

void blit(uint16_t *dest, int destwidth, uint16_t *src, int width, int height, int pitch_pix)
{
	int i, spansz = width << 1;
	for(i=0; i<height; i++) {
		memcpy(dest, src, spansz);
		dest += destwidth;
		src += pitch_pix;
	}
}

void blit_key(uint16_t *dest, int destwidth, uint16_t *src, int width, int height, int pitch_pix, uint16_t key)
{
	int i, j;
	int dadv = destwidth - width;
	int sadv = pitch_pix - width;

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			uint16_t scol = *src++;
			if(scol != key) *dest = scol;
			dest++;
		}
		dest += dadv;
		src += sadv;
	}

}
