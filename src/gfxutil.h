#ifndef GFXUTIL_H_
#define GFXUTIL_H_

#include "inttypes.h"

#define PACK_RGB16(r, g, b) \
	((((uint16_t)(r) << 8) & 0xf800) | \
	 (((uint16_t)(g) << 3) & 0x7e0) | \
	 (((uint16_t)(b) >> 3) & 0x1f))

#define UNPACK_R16(c)	(((c) >> 8) & 0xf8)
#define UNPACK_G16(c)	(((c) >> 3) & 0xfc)
#define UNPACK_B16(c)	(((c) << 3) & 0xf8)

#define PACK_RGB32(r, g, b) \
	((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))

#define UNPACK_R32(c)	(((c) >> 16) & 0xff)
#define UNPACK_G32(c)	(((c) >> 8) & 0xff)
#define UNPACK_B32(c)	((c) & 0xff)

int clip_line(int *x0, int *y0, int *x1, int *y1, int xmin, int ymin, int xmax, int ymax);
void draw_line(int x0, int y0, int x1, int y1, unsigned short color);

/* scale in 24.8 fixed point */
void blur_grey_horiz(uint16_t *dest, uint16_t *src, int xsz, int ysz, int radius, int scale);
void blur_grey_vert(uint16_t *dest, uint16_t *src, int xsz, int ysz, int radius, int scale);

void convimg_rgb24_rgb16(uint16_t *dest, unsigned char *src, int xsz, int ysz);

void blit(uint16_t *dest, int destwidth, uint16_t *src, int xsz, int ysz, int pitch_pix);
void blit_key(uint16_t *dest, int destwidth, uint16_t *src, int xsz, int ysz, int pitch_pix, uint16_t key);

#endif	/* GFXUTIL_H_ */
