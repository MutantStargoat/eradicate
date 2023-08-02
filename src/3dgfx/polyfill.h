#ifndef POLYFILL_H_
#define POLYFILL_H_

#include "inttypes.h"
#include "3dgfx.h"

#define POLYFILL_MODE_MASK	0x03
#define POLYFILL_TEX_BIT	0x04
#define POLYFILL_ALPHA_BIT	0x08
#define POLYFILL_ADD_BIT	0x10
#define POLYFILL_ZBUF_BIT	0x20

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
	POLYFILL_ADD_TEX_GOURAUD,


	POLYFILL_WIRE_ZBUF			= 32,
	POLYFILL_FLAT_ZBUF,
	POLYFILL_GOURAUD_ZBUF,

	POLYFILL_TEX_WIRE_ZBUF		= 36,
	POLYFILL_TEX_FLAT_ZBUF,
	POLYFILL_TEX_GOURAUD_ZBUF,

	POLYFILL_ALPHA_WIRE_ZBUF	= 40,
	POLYFILL_ALPHA_FLAT_ZBUF,
	POLYFILL_ALPHA_GOURAUD_ZBUF,

	POLYFILL_ALPHA_TEX_WIRE_ZBUF = 44,
	POLYFILL_ALPHA_TEX_FLAT_ZBUF,
	POLYFILL_ALPHA_TEX_GOURAUD_ZBUF,

	POLYFILL_ADD_WIRE_ZBUF		= 48,
	POLYFILL_ADD_FLAT_ZBUF,
	POLYFILL_ADD_GOURAUD_ZBUF,

	POLYFILL_ADD_TEX_WIRE_ZBUF	= 52,
	POLYFILL_ADD_TEX_FLAT_ZBUF,
	POLYFILL_ADD_TEX_GOURAUD_ZBUF
};

/* projected vertices for the rasterizer */
struct pvertex {
	int32_t x, y; /* 24.8 fixed point */
	int32_t u, v; /* 16.16 fixed point */
	int32_t r, g, b, a;  /* int 0-255 */
	int32_t z;	/* 0-65535 */
};

struct pimage {
	g3d_pixel *pixels;
	int width, height;

	int xshift, yshift;
	unsigned int xmask, ymask;
};

extern struct pimage pfill_fb;
extern struct pimage pfill_tex;
extern uint16_t *pfill_zbuf;

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
void polyfill_flat_zbuf(struct pvertex *verts, int nverts);
void polyfill_gouraud_zbuf(struct pvertex *verts, int nverts);
void polyfill_tex_flat_zbuf(struct pvertex *verts, int nverts);
void polyfill_tex_gouraud_zbuf(struct pvertex *verts, int nverts);
void polyfill_alpha_flat_zbuf(struct pvertex *verts, int nverts);
void polyfill_alpha_gouraud_zbuf(struct pvertex *verts, int nverts);
void polyfill_alpha_tex_flat_zbuf(struct pvertex *verts, int nverts);
void polyfill_alpha_tex_gouraud_zbuf(struct pvertex *verts, int nverts);
void polyfill_add_flat_zbuf(struct pvertex *verts, int nverts);
void polyfill_add_gouraud_zbuf(struct pvertex *verts, int nverts);
void polyfill_add_tex_flat_zbuf(struct pvertex *verts, int nverts);
void polyfill_add_tex_gouraud_zbuf(struct pvertex *verts, int nverts);

#endif	/* POLYFILL_H_ */
