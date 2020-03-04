#ifndef POLYFILL_H_
#define POLYFILL_H_

#include "inttypes.h"
#include "3dgfx.h"

#define POLYFILL_MODE_MASK	0x03
#define POLYFILL_TEX_BIT	0x04
#define POLYFILL_BLEND_BIT	0x08

enum {
	POLYFILL_WIRE			= 0,
	POLYFILL_FLAT,
	POLYFILL_GOURAUD,

	POLYFILL_TEX_WIRE		= 4,
	POLYFILL_TEX_FLAT,
	POLYFILL_TEX_GOURAUD,

	POLYFILL_BLEND_WIRE		= 8,
	POLYFILL_BLEND_FLAT,
	POLYFILL_BLEND_GOURAUD,

	POLYFILL_BLEND_TEX_WIRE	= 12,
	POLYFILL_BLEND_TEX_FLAT,
	POLYFILL_BLEND_TEX_GOURAUD
};

/* projected vertices for the rasterizer */
struct pvertex {
	int32_t x, y; /* 24.8 fixed point */
	int32_t u, v; /* 16.16 fixed point */
	int32_t r, g, b, a;  /* int 0-255 */
};

struct pimage {
	g3d_pixel *pixels;
	int width, height;

	int xshift, yshift;
	unsigned int xmask, ymask;
};

extern struct pimage pfill_fb;
extern struct pimage pfill_tex;

void polyfill(int mode, struct pvertex *verts, int nverts);

void polyfill_wire(struct pvertex *verts, int nverts);
void polyfill_flat(struct pvertex *verts, int nverts);
void polyfill_gouraud(struct pvertex *verts, int nverts);
void polyfill_tex_wire(struct pvertex *verts, int nverts);
void polyfill_tex_flat(struct pvertex *verts, int nverts);
void polyfill_tex_gouraud(struct pvertex *verts, int nverts);
void polyfill_blend_wire(struct pvertex *verts, int nverts);
void polyfill_blend_flat(struct pvertex *verts, int nverts);
void polyfill_blend_gouraud(struct pvertex *verts, int nverts);
void polyfill_blend_tex_wire(struct pvertex *verts, int nverts);
void polyfill_blend_tex_flat(struct pvertex *verts, int nverts);
void polyfill_blend_tex_gouraud(struct pvertex *verts, int nverts);

#endif	/* POLYFILL_H_ */
