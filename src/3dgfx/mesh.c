#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mesh.h"
#include "3dgfx.h"

void free_mesh(struct g3d_mesh *mesh)
{
	destroy_mesh(mesh);
	free(mesh);
}

void destroy_mesh(struct g3d_mesh *mesh)
{
	free(mesh->varr);
	free(mesh->iarr);
}

int copy_mesh(struct g3d_mesh *dest, struct g3d_mesh *src)
{
	dest->prim = src->prim;
	if(src->varr) {
		if(!(dest->varr = malloc(src->vcount * sizeof *src->varr))) {
			return -1;
		}
		memcpy(dest->varr, src->varr, src->vcount * sizeof *src->varr);
	}
	dest->vcount = src->vcount;
	if(src->iarr) {
		if(!(dest->iarr = malloc(src->icount * sizeof *src->iarr))) {
			free(dest->varr);
			dest->varr = 0;
			return -1;
		}
		memcpy(dest->iarr, src->iarr, src->icount * sizeof *src->iarr);
	}
	dest->icount = src->icount;
	return 0;
}

static struct {
	int prim;
	struct g3d_vertex *varr;
	const float *xform;
} zsort_cls;

static int zsort_cmp(const void *aptr, const void *bptr)
{
	int i;
	float za = 0.0f;
	float zb = 0.0f;
	const float *m = zsort_cls.xform;
	const struct g3d_vertex *va = (const struct g3d_vertex*)aptr;
	const struct g3d_vertex *vb = (const struct g3d_vertex*)bptr;

	for(i=0; i<zsort_cls.prim; i++) {
		za += m[2] * va->x + m[6] * va->y + m[10] * va->z + m[14];
		zb += m[2] * vb->x + m[6] * vb->y + m[10] * vb->z + m[14];
		++va;
		++vb;
	}

	za -= zb;
	return *(int*)&za;
}

static int zsort_indexed_cmp(const void *aptr, const void *bptr)
{
	int i;
	float za = 0.0f;
	float zb = 0.0f;
	const uint16_t *a = (const uint16_t*)aptr;
	const uint16_t *b = (const uint16_t*)bptr;

	const float *m = zsort_cls.xform;

	for(i=0; i<zsort_cls.prim; i++) {
		const struct g3d_vertex *va = zsort_cls.varr + a[i];
		const struct g3d_vertex *vb = zsort_cls.varr + b[i];

		za += m[2] * va->x + m[6] * va->y + m[10] * va->z + m[14];
		zb += m[2] * vb->x + m[6] * vb->y + m[10] * vb->z + m[14];
	}

	za -= zb;
	return *(int*)&za;
}


void zsort_mesh(struct g3d_mesh *m)
{
	zsort_cls.varr = m->varr;
	zsort_cls.xform = g3d_get_matrix(G3D_MODELVIEW, 0);
	zsort_cls.prim = m->prim;

	if(m->iarr) {
		int nfaces = m->icount / m->prim;
		qsort(m->iarr, nfaces, m->prim * sizeof *m->iarr, zsort_indexed_cmp);
	} else {
		int nfaces = m->vcount / m->prim;
		qsort(m->varr, nfaces, m->prim * sizeof *m->varr, zsort_cmp);
	}
}


void draw_mesh(struct g3d_mesh *mesh)
{
	if(mesh->iarr) {
		g3d_draw_indexed(mesh->prim, mesh->varr, mesh->vcount, mesh->iarr, mesh->icount);
	} else {
		g3d_draw(mesh->prim, mesh->varr, mesh->vcount);
	}
}

void apply_mesh_xform(struct g3d_mesh *mesh, const float *xform)
{
	int i;
	struct g3d_vertex *v = mesh->varr;

	for(i=0; i<mesh->vcount; i++) {
		float x = xform[0] * v->x + xform[4] * v->y + xform[8] * v->z + xform[12];
		float y = xform[1] * v->x + xform[5] * v->y + xform[9] * v->z + xform[13];
		v->z = xform[2] * v->x + xform[6] * v->y + xform[10] * v->z + xform[14];
		v->x = x;
		v->y = y;
		x = xform[0] * v->nx + xform[4] * v->ny + xform[8] * v->nz;
		y = xform[1] * v->nx + xform[5] * v->ny + xform[9] * v->nz;
		v->nz = xform[2] * v->nx + xform[6] * v->ny + xform[10] * v->nz;
		v->nx = x;
		v->ny = y;
		++v;
	}
}

int append_mesh(struct g3d_mesh *ma, struct g3d_mesh *mb)
{
	int i, new_vcount, new_icount;
	void *tmp;
	uint16_t *iptr;

	if(ma->prim != mb->prim) {
		fprintf(stderr, "append_mesh failed, primitive mismatch\n");
		return -1;
	}

	if(ma->iarr || mb->iarr) {
		if(!ma->iarr) {
			if(indexify_mesh(ma) == -1) {
				return -1;
			}
		} else if(!mb->iarr) {
			if(indexify_mesh(mb) == -1) {
				return -1;
			}
		}

		new_icount = ma->icount + mb->icount;
		if(!(iptr = realloc(ma->iarr, new_icount * sizeof *iptr))) {
			fprintf(stderr, "append_mesh: failed to allocate combined index buffer (%d indices)\n", new_icount);
			return -1;
		}
		ma->iarr = iptr;

		iptr += ma->icount;
		for(i=0; i<mb->icount; i++) {
			*iptr++ = mb->iarr[i] + ma->vcount;
		}
		ma->icount = new_icount;
	}

	new_vcount = ma->vcount + mb->vcount;
	if(!(tmp = realloc(ma->varr, new_vcount * sizeof *ma->varr))) {
		fprintf(stderr, "append_mesh: failed to allocate combined vertex buffer (%d verts)\n", new_vcount);
		return -1;
	}
	ma->varr = tmp;
	memcpy(ma->varr + ma->vcount, mb->varr, mb->vcount * sizeof *ma->varr);
	ma->vcount = new_vcount;
	return 0;
}

#define FEQ(a, b)	((a) - (b) < 1e-5 && (b) - (a) < 1e-5)
static int cmp_vertex(struct g3d_vertex *a, struct g3d_vertex *b)
{
	if(!FEQ(a->x, b->x) || !FEQ(a->y, b->y) || !FEQ(a->z, b->z) || !FEQ(a->w, b->w))
		return -1;
	if(!FEQ(a->nx, b->nx) || !FEQ(a->ny, b->ny) || !FEQ(a->nz, b->nz))
		return -1;
	if(!FEQ(a->u, b->u) || !FEQ(a->v, b->v))
		return -1;
	if(a->r != b->r || a->g != b->g || a->b != b->b || a->a != b->a)
		return -1;
	return 0;
}

static int find_existing(struct g3d_vertex *v, struct g3d_vertex *varr, int vcount)
{
	int i;
	for(i=0; i<vcount; i++) {
		if(cmp_vertex(v, varr++) == 0) {
			return i;
		}
	}
	return -1;
}

int indexify_mesh(struct g3d_mesh *mesh)
{
	int i, j, nfaces, max_icount, idx;
	int out_vcount = 0;
	struct g3d_vertex *vin, *vout;
	uint16_t *iout;

	if(mesh->iarr) {
		fprintf(stderr, "indexify_mesh failed: already indexed\n");
		return -1;
	}

	nfaces = mesh->vcount / mesh->prim;
	max_icount = mesh->vcount;

	if(!(mesh->iarr = malloc(max_icount * sizeof *mesh->iarr))) {
		fprintf(stderr, "indexify_mesh failed to allocate index buffer of %d indices\n", max_icount);
		return -1;
	}

	vin = vout = mesh->varr;
	iout = mesh->iarr;

	for(i=0; i<nfaces; i++) {
		for(j=0; j<mesh->prim; j++) {
			if((idx = find_existing(vin, mesh->varr, out_vcount)) >= 0) {
				*iout++ = idx;
			} else {
				*iout++ = out_vcount++;
				if(vin != vout) {
					*vout++ = *vin;
				}
			}
			++vin;
		}
	}

	/* XXX also shrink buffers? I'll just leave them to max size for now */
	return 0;
}

void normalize_mesh_normals(struct g3d_mesh *mesh)
{
	int i;
	struct g3d_vertex *v = mesh->varr;

	for(i=0; i<mesh->vcount; i++) {
		float mag = sqrt(v->nx * v->nx + v->ny * v->ny + v->nz * v->nz);
		float s = (mag == 0.0f) ? 1.0f : 1.0f / mag;
		v->nx *= s;
		v->ny *= s;
		v->nz *= s;
		++v;
	}
}


void calc_mesh_centroid(struct g3d_mesh *mesh, float *cent)
{
	int i;
	float s = 1.0f / (float)mesh->vcount;
	cent[0] = cent[1] = cent[2] = 0.0f;

	for(i=0; i<mesh->vcount; i++) {
		cent[0] += mesh->varr[i].x;
		cent[1] += mesh->varr[i].y;
		cent[2] += mesh->varr[i].z;
	}
	cent[0] *= s;
	cent[1] *= s;
	cent[2] *= s;
}

static void sphvec(float *res, float theta, float phi, float rad)
{
	theta = -theta;
	res[0] = sin(theta) * sin(phi);
	res[1] = cos(phi);
	res[2] = cos(theta) * sin(phi);
}

int gen_sphere_mesh(struct g3d_mesh *mesh, float rad, int usub, int vsub)
{
	int i, j;
	int nfaces, uverts, vverts;
	struct g3d_vertex *vptr;
	uint16_t *iptr;

	mesh->prim = G3D_QUADS;

	if(usub < 4) usub = 4;
	if(vsub < 2) vsub = 2;

	uverts = usub + 1;
	vverts = vsub + 1;

	mesh->vcount = uverts * vverts;
	nfaces = usub * vsub;
	mesh->icount = nfaces * 4;

	if(!(mesh->varr = malloc(mesh->vcount * sizeof *mesh->varr))) {
		fprintf(stderr, "gen_sphere_mesh: failed to allocate vertex buffer (%d vertices)\n", mesh->vcount);
		return -1;
	}
	if(!(mesh->iarr = malloc(mesh->icount * sizeof *mesh->iarr))) {
		fprintf(stderr, "gen_sphere_mesh: failed to allocate index buffer (%d indices)\n", mesh->icount);
		return -1;
	}
	vptr = mesh->varr;
	iptr = mesh->iarr;

	for(i=0; i<uverts; i++) {
		float u = (float)i / (float)(uverts - 1);
		float theta = u * 2.0 * M_PI;

		for(j=0; j<vverts; j++) {
			float v = (float)j / (float)(vverts - 1);
			float phi = v * M_PI;
			int chess = (i & 1) == (j & 1);

			sphvec(&vptr->x, theta, phi, rad);
			vptr->w = 1.0f;

			vptr->nx = vptr->x / rad;
			vptr->ny = vptr->y / rad;
			vptr->nz = vptr->z / rad;
			vptr->u = u;
			vptr->v = v;
			vptr->r = chess ? 255 : 64;
			vptr->g = 128;
			vptr->b = chess ? 64 : 255;
			++vptr;

			if(i < usub && j < vsub) {
				int idx = i * vverts + j;
				*iptr++ = idx;
				*iptr++ = idx + 1;
				*iptr++ = idx + vverts + 1;
				*iptr++ = idx + vverts;
			}
		}
	}
	return 0;
}

int gen_plane_mesh(struct g3d_mesh *m, float width, float height, int usub, int vsub)
{
	int i, j;
	int nfaces, nverts, nidx, uverts, vverts;
	float x, y, u, v, du, dv;
	struct g3d_vertex *vptr;
	uint16_t *iptr;

	if(usub < 1) usub = 1;
	if(vsub < 1) vsub = 1;

	nfaces = usub * vsub;
	uverts = usub + 1;
	vverts = vsub + 1;
	du = (float)width / (float)usub;
	dv = (float)height / (float)vsub;

	nverts = uverts * vverts;
	nidx = nfaces * 4;

	if(!(m->varr = malloc(nverts * sizeof *m->varr))) {
		fprintf(stderr, "gen_plane_mesh: failed to allocate vertex buffer (%d vertices)\n", nverts);
		return -1;
	}
	if(!(m->iarr = malloc(nidx * sizeof *m->iarr))) {
		fprintf(stderr, "gen_plane_mesh: failed to allocate index buffer (%d indices)\n", nidx);
		free(m->varr);
		m->varr = 0;
		return -1;
	}

	m->prim = G3D_QUADS;
	m->vcount = nverts;
	m->icount = nidx;

	vptr = m->varr;
	iptr = m->iarr;

	v = 0.0f;
	for(i=0; i<vverts; i++) {
		y = (v - 0.5) * height;
		u = 0.0f;

		for(j=0; j<uverts; j++) {
			x = (u - 0.5) * width;

			vptr->x = x;
			vptr->y = y;
			vptr->z = 0.0f;
			vptr->w = 1.0f;
			vptr->nx = 0.0f;
			vptr->ny = 0.0f;
			vptr->nz = 1.0f;
			vptr->u = u;
			vptr->v = v;
			vptr->r = vptr->g = vptr->b = vptr->a = 255;
			++vptr;

			u += du;
		}
		v += dv;
	}

	for(i=0; i<vsub; i++) {
		for(j=0; j<usub; j++) {
			int idx = i * uverts + j;
			*iptr++ = idx;
			*iptr++ = idx + 1;
			*iptr++ = idx + uverts + 1;
			*iptr++ = idx + uverts;
		}
	}
	return 0;
}

int gen_cube_mesh(struct g3d_mesh *mesh, float sz, int sub)
{
	int i;
	float offs;
	struct g3d_mesh *m;
	struct g3d_mesh tmpmesh;
	static float rotface[][4] = {
		{0, 0, 1, 0},
		{90, 0, 1, 0},
		{180, 0, 1, 0},
		{270, 0, 1, 0},
		{90, 1, 0, 0},
		{-90, 1, 0, 0}
	};

	offs = sz;
	sz = fabs(sz);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_push_matrix();

	for(i=0; i<6; i++) {
		m = i > 0 ? &tmpmesh : mesh;
		if(gen_plane_mesh(m, sz, sz, sub, sub) == -1)
			return -1;
		g3d_load_identity();
		g3d_rotate(rotface[i][0], rotface[i][1], rotface[i][2], rotface[i][3]);
		g3d_translate(0, 0, offs / 2.0f);
		apply_mesh_xform(m, g3d_get_matrix(G3D_MODELVIEW, 0));
		if(i > 0) {
			if(append_mesh(mesh, m) == -1) {
				return -1;
			}
		}
	}

	g3d_pop_matrix();
	return 0;
}

static void torusvec(float *res, float theta, float phi, float mr, float rr)
{
	float rx, ry, rz;
	theta = -theta;

	rx = -cos(phi) * rr + mr;
	ry = sin(phi) * rr;
	rz = 0.0f;

	res[0] = rx * sin(theta) + rz * cos(theta);
	res[1] = ry;
	res[2] = -rx * cos(theta) + rz * sin(theta);
}

int gen_torus_mesh(struct g3d_mesh *mesh, float rad, float ringrad, int usub, int vsub)
{
	int i, j;
	int nfaces, uverts, vverts;
	struct g3d_vertex *vptr;
	uint16_t *iptr;

	mesh->prim = G3D_QUADS;

	if(usub < 4) usub = 4;
	if(vsub < 2) vsub = 2;

	uverts = usub + 1;
	vverts = vsub + 1;

	mesh->vcount = uverts * vverts;
	nfaces = usub * vsub;
	mesh->icount = nfaces * 4;

	printf("generating torus with %d faces (%d vertices)\n", nfaces, mesh->vcount);

	if(!(mesh->varr = malloc(mesh->vcount * sizeof *mesh->varr))) {
		return -1;
	}
	if(!(mesh->iarr = malloc(mesh->icount * sizeof *mesh->iarr))) {
		return -1;
	}
	vptr = mesh->varr;
	iptr = mesh->iarr;

	for(i=0; i<uverts; i++) {
		float u = (float)i / (float)(uverts - 1);
		float theta = u * 2.0 * M_PI;
		float rcent[3];

		torusvec(rcent, theta, 0, rad, 0);

		for(j=0; j<vverts; j++) {
			float v = (float)j / (float)(vverts - 1);
			float phi = v * 2.0 * M_PI;
			int chess = (i & 1) == (j & 1);

			torusvec(&vptr->x, theta, phi, rad, ringrad);
			vptr->w = 1.0f;

			vptr->nx = (vptr->x - rcent[0]) / ringrad;
			vptr->ny = (vptr->y - rcent[1]) / ringrad;
			vptr->nz = (vptr->z - rcent[2]) / ringrad;
			vptr->u = u;
			vptr->v = v;
			vptr->r = chess ? 255 : 64;
			vptr->g = 128;
			vptr->b = chess ? 64 : 255;
			++vptr;

			if(i < usub && j < vsub) {
				int idx = i * vverts + j;
				*iptr++ = idx;
				*iptr++ = idx + 1;
				*iptr++ = idx + vverts + 1;
				*iptr++ = idx + vverts;
			}
		}
	}
	return 0;
}

