#ifndef POLYFILL_H_
#define POLYFILL_H_

#include "inttypes.h"
#include "3dgfx.h"

#define POLYFILL_MODE_MASK	0x03
#define POLYFILL_TEX_BIT	0x04
#define POLYFILL_ALPHA_BIT	0x08
#define POLYFILL_ADD_BIT	0x10

enum {
	POLYFILL_WIRE			= 0,
	POLYFILL_FLAT,
	POLYFILL_GOURAUD,

	POLYFILL_TEX_WIRE		= 4,
	POLYFILL_TEX_FLAT,
	POLYFILL_TEX_GOURAUD,

	POLYFILL_ALPHA_WIRE		= 8,
	POLYFILL_ALPHA_FLAT,
	POLYFILL_ALPHA_GOURAUD,

	POLYFILL_ALPHA_TEX_WIRE	= 12,
	POLYFILL_ALPHA_TEX_FLAT,
	POLYFILL_ALPHA_TEX_GOURAUD,

	POLYFILL_ADD_WIRE		= 16,
	POLYFILL_ADD_FLAT,
	POLYFILL_ADD_GOURAUD,

	POLYFILL_ADD_TEX_WIRE	= 20,
	POLYFILL_ADD_TEX_FLAT,
	POLYFILL_ADD_TEX_GOURAUD
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

void polyfill_fbheight(int height);

void polyfill(int mode, struct pvertex *verts, int nverts);

void polyfill_wire(struct pvertex *verts, int nverts);
void polyfill_flat(struct pvertex *verts, int nverts);
void polyfill_gouraud(struct pvertex *verts, int nverts);
void polyfill_tex_wire(struct pvertex *verts, int nverts);
void polyfill_tex_flat(struct pvertex *verts, int nverts);
void polyfill_tex_gouraud(struct pvertex *verts, int nverts);
void polyfill_alpha_wire(struct pvertex *verts, int nverts);
void polyfill_alpha_flat(struct pvertex *verts, int nverts);
void polyfill_alpha_gouraud(struct pvertex *verts, int nverts);
void polyfill_alpha_tex_wire(struct pvertex *verts, int nverts);
void polyfill_alpha_tex_flat(struct pvertex *verts, int nverts);
void polyfill_alpha_tex_gouraud(struct pvertex *verts, int nverts);
void polyfill_add_wire(struct pvertex *verts, int nverts);
void polyfill_add_flat(struct pvertex *verts, int nverts);
void polyfill_add_gouraud(struct pvertex *verts, int nverts);
void polyfill_add_tex_wire(struct pvertex *verts, int nverts);
void polyfill_add_tex_flat(struct pvertex *verts, int nverts);
void polyfill_add_tex_gouraud(struct pvertex *verts, int nverts);

#endif	/* POLYFILL_H_ */
