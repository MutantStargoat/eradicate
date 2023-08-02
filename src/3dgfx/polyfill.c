#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "polyfill.h"
#include "gfxutil.h"
#include "util.h"

/*#define DEBUG_OVERDRAW	G3D_PACK_RGB(10, 10, 10)*/

#define FILL_POLY_BITS	0x03

void polyfill_flat_new(struct pvertex *varr, int vnum);

/* mode bits: 00-wire 01-flat 10-gouraud 11-reserved
 *     bit 2: texture
 *     bit 3-4: blend mode: 00-none 01-alpha 10-additive 11-reserved
 *     bit 5: zbuffering
 */
void (*fillfunc[])(struct pvertex*, int) = {
	polyfill_wire,
	polyfill_flat,
	polyfill_gouraud,
	0,
	polyfill_tex_wire,
	polyfill_tex_flat,
	polyfill_tex_gouraud,
	0,
	polyfill_alpha_wire,
	polyfill_alpha_flat,
	polyfill_alpha_gouraud,
	0,
	polyfill_alpha_tex_wire,
	polyfill_alpha_tex_flat,
	polyfill_alpha_tex_gouraud,
	0,
	polyfill_add_wire,
	polyfill_add_flat,
	polyfill_add_gouraud,
	0,
	polyfill_add_tex_wire,
	polyfill_add_tex_flat,
	polyfill_add_tex_gouraud,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	polyfill_wire,
	polyfill_flat_zbuf,
	polyfill_gouraud_zbuf,
	0,
	polyfill_tex_wire,
	polyfill_tex_flat_zbuf,
	polyfill_tex_gouraud_zbuf,
	0,
	polyfill_alpha_wire,
	polyfill_alpha_flat_zbuf,
	polyfill_alpha_gouraud_zbuf,
	0,
	polyfill_alpha_tex_wire,
	polyfill_alpha_tex_flat_zbuf,
	polyfill_alpha_tex_gouraud_zbuf,
	0,
	polyfill_add_wire,
	polyfill_add_flat_zbuf,
	polyfill_add_gouraud_zbuf,
	0,
	polyfill_add_tex_wire,
	polyfill_add_tex_flat_zbuf,
	polyfill_add_tex_gouraud_zbuf,
	0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct pimage pfill_fb, pfill_tex;
uint16_t *pfill_zbuf;

#define EDGEPAD	8
static struct pvertex *edgebuf, *left, *right;
static int edgebuf_size;
static int fbheight;

/*
#define CHECKEDGE(x) \
	do { \
		assert(x >= 0); \
		assert(x < fbheight); \
	} while(0)
*/
#define CHECKEDGE(x)


void polyfill_fbheight(int height)
{
	int newsz = (height * 2 + EDGEPAD * 3) * sizeof *edgebuf;

	if(newsz > edgebuf_size) {
		free(edgebuf);
		if(!(edgebuf = malloc(newsz))) {
			fprintf(stderr, "failed to allocate edge table buffer (%d bytes)\n", newsz);
			abort();
		}
		edgebuf_size = newsz;

		left = edgebuf + EDGEPAD;
		right = edgebuf + height + EDGEPAD * 2;

#ifndef NDEBUG
		memset(edgebuf, 0xaa, EDGEPAD * sizeof *edgebuf);
		memset(edgebuf + height + EDGEPAD, 0xaa, EDGEPAD * sizeof *edgebuf);
		memset(edgebuf + height * 2 + EDGEPAD * 2, 0xaa, EDGEPAD * sizeof *edgebuf);
#endif
	}

	fbheight = height;
}

void polyfill(int mode, struct pvertex *verts, int nverts)
{
#ifndef NDEBUG
	if(!fillfunc[mode]) {
		fprintf(stderr, "polyfill mode %d not implemented\n", mode);
		abort();
	}
#endif

	fillfunc[mode](verts, nverts);
}

void polyfill_wire(struct pvertex *verts, int nverts)
{
	int i, x0, y0, x1, y1;
	struct pvertex *v = verts;
	unsigned short color = ((v->r << 8) & 0xf800) |
		((v->g << 3) & 0x7e0) | ((v->b >> 3) & 0x1f);

	for(i=0; i<nverts - 1; i++) {
		x0 = v->x >> 8;
		y0 = v->y >> 8;
		++v;
		x1 = v->x >> 8;
		y1 = v->y >> 8;
		if(clip_line(&x0, &y0, &x1, &y1, 0, 0, pfill_fb.width, pfill_fb.height)) {
			draw_line(x0, y0, x1, y1, color);
		}
	}
	x0 = verts[0].x >> 8;
	y0 = verts[0].y >> 8;
	if(clip_line(&x1, &y1, &x0, &y0, 0, 0, pfill_fb.width, pfill_fb.height)) {
		draw_line(x1, y1, x0, y0, color);
	}
}

void polyfill_tex_wire(struct pvertex *verts, int nverts)
{
	polyfill_wire(verts, nverts);	/* TODO */
}

void polyfill_alpha_wire(struct pvertex *verts, int nverts)
{
	polyfill_wire(verts, nverts);	/* TODO */
}

void polyfill_alpha_tex_wire(struct pvertex *verts, int nverts)
{
	polyfill_wire(verts, nverts);	/* TODO */
}

void polyfill_add_wire(struct pvertex *verts, int nverts)
{
	polyfill_wire(verts, nverts);	/* TODO */
}

void polyfill_add_tex_wire(struct pvertex *verts, int nverts)
{
	polyfill_wire(verts, nverts);	/* TODO */
}

#define VNEXT(p)	(((p) == vlast) ? varr : (p) + 1)
#define VPREV(p)	((p) == varr ? vlast : (p) - 1)
#define VSUCC(p, side)	((side) == 0 ? VNEXT(p) : VPREV(p))

/* extra bits of precision to use when interpolating colors.
 * try tweaking this if you notice strange quantization artifacts.
 */
#define COLOR_SHIFT	12


#define POLYFILL polyfill_flat
#undef GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_gouraud
#define GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_flat
#undef GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_gouraud
#define GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_flat
#undef GOURAUD
#undef TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_gouraud
#define GOURAUD
#undef TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_tex_flat
#undef GOURAUD
#define TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_tex_gouraud
#define GOURAUD
#define TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_flat
#undef GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_gouraud
#define GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_tex_flat
#undef GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_tex_gouraud
#define GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

/* ---- zbuffer variants ----- */

#define POLYFILL polyfill_flat_zbuf
#undef GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_gouraud_zbuf
#define GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_flat_zbuf
#undef GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_gouraud_zbuf
#define GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_flat_zbuf
#undef GOURAUD
#undef TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_gouraud_zbuf
#define GOURAUD
#undef TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_tex_flat_zbuf
#undef GOURAUD
#define TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_alpha_tex_gouraud_zbuf
#define GOURAUD
#define TEXMAP
#define BLEND_ALPHA
#undef BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_flat_zbuf
#undef GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_gouraud_zbuf
#define GOURAUD
#undef TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_tex_flat_zbuf
#undef GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_add_tex_gouraud_zbuf
#define GOURAUD
#define TEXMAP
#undef BLEND_ALPHA
#define BLEND_ADD
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

