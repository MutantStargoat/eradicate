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
		int sum = sptr[0] * (rad + 1); \
		int count = (rad * 2 + 1) << 8; \
		int midsize = w - rad * 2; \
		int firstpix = sptr[0]; \
		int lastpix = sptr[pstep * (w - 1)]; \
		/* add up the contributions for the -1 pixel */ \
		for(j=0; j<rad; j++) { \
			sum += sptr[pstep * j]; \
		} \
		/* first part adding sptr[rad] and subtracting sptr[0] */ \
		for(j=0; j<=rad; j++) { \
			sum += (int)sptr[pstep * rad] - firstpix; \
			sptr += pstep; \
			*dptr = scale * sum / count; \
			dptr += pstep; \
		} \
		/* middle part adding sptr[rad] and subtracting sptr[-(rad+1)] */ \
		for(j=1; j<midsize; j++) { \
			sum += (int)sptr[pstep * rad] - (int)sptr[-(rad + 1) * pstep]; \
			sptr += pstep; \
			*dptr = scale * sum / count; \
			dptr += pstep; \
		} \
		/* last part adding lastpix and subtracting sptr[-(rad+1)] */ \
		for(j=0; j<rad; j++) { \
			sum += lastpix - (int)sptr[-(rad + 1) * pstep]; \
			sptr += pstep; \
			*dptr = scale * sum / count; \
			dptr += pstep; \
		} \
		sptr += sstep; \
		dptr += sstep; \
	}

/* TODO bound blur rad to image size to avoid inner loop conditionals */
/* TODO make version with pow2 (rad*2+1) to avoid div with count everywhere */
void blur_grey_horiz(uint16_t *dest, uint16_t *src, int xsz, int ysz, int rad, int scale)
{
	int i, j;
	unsigned char *dptr = (unsigned char*)dest;
	unsigned char *sptr = (unsigned char*)src;

	BLUR(xsz, ysz, 2, 0);
}


void blur_grey_vert(uint16_t *dest, uint16_t *src, int xsz, int ysz, int rad, int scale)
{
	int i, j;
	unsigned char *dptr = (unsigned char*)dest;
	unsigned char *sptr = (unsigned char*)src;
	int pixel_step = xsz * 2;
	int scanline_step = 2 - ysz * pixel_step;

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

void blitfb(uint16_t *dest, uint16_t *src, int width, int height, int pitch_pix)
{
	int i;
	for(i=0; i<height; i++) {
		memcpy(dest, src, width * 2);
		dest += 320;
		src += pitch_pix;
	}
}

void blitfb_key(uint16_t *dest, uint16_t *src, int width, int height, int pitch_pix, uint16_t key)
{
	int i, j, dadv = 320 - width;

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			uint16_t scol = *src++;
			if(scol != key) *dest = scol;
			dest++;
		}
		dest += dadv;
	}

}
