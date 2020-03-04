#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "mesh.h"
#include "dynarr.h"
#include "rbtree.h"
#include "3dgfx.h"
#include "util.h"

typedef struct { float x, y; } vec2_t;
typedef struct { float x, y, z; } vec3_t;
typedef struct { float x, y, z, w; } vec4_t;


struct vertex_pos_color {
	float x, y, z;
	float r, g, b, a;
};

struct facevertex {
	int vidx, tidx, nidx;
};

static char *clean_line(char *s);
static char *parse_face_vert(char *ptr, struct facevertex *fv, int numv, int numt, int numn);
static int cmp_facevert(const void *ap, const void *bp);
static void free_rbnode_key(struct rbnode *n, void *cls);

/* merge of different indices per attribute happens during face processing.
 *
 * A triplet of (vertex index/texcoord index/normal index) is used as the key
 * to search in a balanced binary search tree for vertex buffer index assigned
 * to the same triplet if it has been encountered before. That index is
 * appended to the index buffer.
 *
 * If a particular triplet has not been encountered before, a new g3d_vertex is
 * appended to the vertex buffer. The index of this new vertex is appended to
 * the index buffer, and also inserted into the tree for future searches.
 */
int load_mesh(struct g3d_mesh *mesh, const char *fname)
{
	int i, line_num = 0, result = -1;
	int found_quad = 0;
	FILE *fp = 0;
	char buf[256];
	struct vertex_pos_color *varr = 0;
	vec3_t *narr = 0;
	vec2_t *tarr = 0;
	struct rbtree *rbtree = 0;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "load_mesh: failed to open file: %s\n", fname);
		goto err;
	}

	if(!(rbtree = rb_create(cmp_facevert))) {
		fprintf(stderr, "load_mesh: failed to create facevertex binary search tree\n");
		goto err;
	}
	rb_set_delete_func(rbtree, free_rbnode_key, 0);

	if(!(mesh->varr = dynarr_alloc(0, sizeof *mesh->varr)) ||
			!(mesh->iarr = dynarr_alloc(0, sizeof *mesh->iarr))) {
		fprintf(stderr, "load_mesh: failed to allocate resizable mesh arrays\n");
		goto err;
	}
	if(!(varr = dynarr_alloc(0, sizeof *varr)) ||
			!(narr = dynarr_alloc(0, sizeof *narr)) ||
			!(tarr = dynarr_alloc(0, sizeof *tarr))) {
		fprintf(stderr, "load_mesh: failed to allocate resizable vertex array\n");
		goto err;
	}

	while(fgets(buf, sizeof buf, fp)) {
		char *line = clean_line(buf);
		++line_num;

		if(!*line) continue;

		switch(line[0]) {
		case 'v':
			if(isspace(line[1])) {
				/* vertex */
				struct vertex_pos_color v;
				int num;

				num = sscanf(line + 2, "%f %f %f %f %f %f %f", &v.x, &v.y, &v.z, &v.r, &v.g, &v.b, &v.a);
				if(num < 3) {
					fprintf(stderr, "%s:%d: invalid vertex definition: \"%s\"\n", fname, line_num, line);
					goto err;
				}
				switch(num) {
				case 3:
					v.r = 1.0f;
				case 4:
					v.g = 1.0f;
				case 5:
					v.b = 1.0f;
				case 6:
					v.a = 1.0f;
				}
				if(!(varr = dynarr_push(varr, &v))) {
					fprintf(stderr, "load_mesh: failed to resize vertex buffer\n");
					goto err;
				}

			} else if(line[1] == 't' && isspace(line[2])) {
				/* texcoord */
				vec2_t tc;
				if(sscanf(line + 3, "%f %f", &tc.x, &tc.y) != 2) {
					fprintf(stderr, "%s:%d: invalid texcoord definition: \"%s\"\n", fname, line_num, line);
					goto err;
				}
				if(!(tarr = dynarr_push(tarr, &tc))) {
					fprintf(stderr, "load_mesh: failed to resize texcoord buffer\n");
					goto err;
				}

			} else if(line[1] == 'n' && isspace(line[2])) {
				/* normal */
				vec3_t norm;
				if(sscanf(line + 3, "%f %f %f", &norm.x, &norm.y, &norm.z) != 3) {
					fprintf(stderr, "%s:%d: invalid normal definition: \"%s\"\n", fname, line_num, line);
					goto err;
				}
				if(!(narr = dynarr_push(narr, &norm))) {
					fprintf(stderr, "load_mesh: failed to resize normal buffer\n");
					goto err;
				}
			}
			break;

		case 'f':
			if(isspace(line[1])) {
				/* face */
				char *ptr = line + 2;
				struct facevertex fv;
				struct rbnode *node;
				int vsz = dynarr_size(varr);
				int tsz = dynarr_size(tarr);
				int nsz = dynarr_size(narr);

				for(i=0; i<4; i++) {
					if(!(ptr = parse_face_vert(ptr, &fv, vsz, tsz, nsz))) {
						if(i < 3 || found_quad) {
							fprintf(stderr, "%s:%d: invalid face definition: \"%s\"\n", fname, line_num, line);
							goto err;
						} else {
							break;
						}
					}

					if((node = rb_find(rbtree, &fv))) {
						uint16_t idx = (int)(intptr_t)node->data;
						if(!(mesh->iarr = dynarr_push(mesh->iarr, &idx))) {
							fprintf(stderr, "load_mesh: failed to resize index array\n");
							goto err;
						}
					} else {
						uint16_t newidx = dynarr_size(mesh->varr);
						struct g3d_vertex v;
						struct facevertex *newfv;

						v.x = varr[fv.vidx].x;
						v.y = varr[fv.vidx].y;
						v.z = varr[fv.vidx].z;
						v.w = 1.0f;
						v.r = cround64(varr[fv.vidx].r * 255.0);
						v.g = cround64(varr[fv.vidx].g * 255.0);
						v.b = cround64(varr[fv.vidx].b * 255.0);
						v.a = cround64(varr[fv.vidx].a * 255.0);
						if(fv.tidx >= 0) {
							v.u = tarr[fv.tidx].x;
							v.v = tarr[fv.tidx].y;
						} else {
							v.u = v.x;
							v.v = v.y;
						}
						if(fv.nidx >= 0) {
							v.nx = narr[fv.nidx].x;
							v.ny = narr[fv.nidx].y;
							v.nz = narr[fv.nidx].z;
						} else {
							v.nx = v.ny = 0.0f;
							v.nz = 1.0f;
						}

						if(!(mesh->varr = dynarr_push(mesh->varr, &v))) {
							fprintf(stderr, "load_mesh: failed to resize combined vertex array\n");
							goto err;
						}
						if(!(mesh->iarr = dynarr_push(mesh->iarr, &newidx))) {
							fprintf(stderr, "load_mesh: failed to resize index array\n");
							goto err;
						}

						if((newfv = malloc(sizeof *newfv))) {
							*newfv = fv;
						}
						if(!newfv || rb_insert(rbtree, newfv, (void*)(intptr_t)newidx) == -1) {
							fprintf(stderr, "load_mesh: failed to insert facevertex to the binary search tree\n");
							goto err;
						}
					}
				}
				if(i > 3) found_quad = 1;
			}
			break;

		default:
			break;
		}
	}

	mesh->prim = found_quad ? G3D_QUADS : G3D_TRIANGLES;
	mesh->vcount = dynarr_size(mesh->varr);
	mesh->icount = dynarr_size(mesh->iarr);
	mesh->varr = dynarr_finalize(mesh->varr);
	mesh->iarr = dynarr_finalize(mesh->iarr);
	result = 0;	/* success */

	printf("loaded %s mesh: %s: %d vertices, %d faces\n", found_quad ? "quad" : "triangle",
			fname, mesh->vcount, mesh->icount / mesh->prim);

err:
	if(fp) fclose(fp);
	dynarr_free(varr);
	dynarr_free(narr);
	dynarr_free(tarr);
	if(result == -1) {
		dynarr_free(mesh->varr);
		dynarr_free(mesh->iarr);
	}
	rb_free(rbtree);
	return result;
}

int save_mesh(struct g3d_mesh *mesh, const char *fname)
{
	int i, fvcount;
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "save_mesh: failed to open %s for writing\n", fname);
		return -1;
	}
	fprintf(fp, "# Wavefront OBJ file shoved in your FACE by Mindlapse. Deal with it\n");

	for(i=0; i<mesh->vcount; i++) {
		struct g3d_vertex *v = mesh->varr + i;
		fprintf(fp, "v %f %f %f %f %f %f %f\n", v->x, v->y, v->z, v->r / 255.0f, v->g / 255.0f,
				v->b / 255.0f, v->a / 255.0f);
	}
	for(i=0; i<mesh->vcount; i++) {
		fprintf(fp, "vn %f %f %f\n", mesh->varr[i].nx, mesh->varr[i].ny, mesh->varr[i].nz);
	}
	for(i=0; i<mesh->vcount; i++) {
		fprintf(fp, "vt %f %f\n", mesh->varr[i].u, mesh->varr[i].v);
	}

	fvcount = mesh->prim;
	for(i=0; i<mesh->icount; i++) {
		int idx = mesh->iarr[i] + 1;

		if(fvcount == mesh->prim) {
			fprintf(fp, "\nf");
			fvcount = 0;
		}
		fprintf(fp, " %d/%d/%d", idx, idx, idx);
		++fvcount;
	}
	fprintf(fp, "\n");

	fclose(fp);
	return 0;
}

static char *clean_line(char *s)
{
	char *end;

	while(*s && isspace(*s)) ++s;
	if(!*s) return 0;

	end = s;
	while(*end && *end != '#') ++end;
	*end = 0;

	while(end > s && isspace(*end)) --end;
	*end = 0;

	return s;
}

static char *parse_idx(char *ptr, int *idx, int arrsz)
{
	char *endp;
	int val = strtol(ptr, &endp, 10);
	if(endp == ptr) return 0;

	if(val < 0) {	/* convert negative indices */
		*idx = arrsz + val;
	} else {
		*idx = val - 1;	/* indices in obj are 1-based */
	}
	return endp;
}

/* possible face-vertex definitions:
 * 1. vertex
 * 2. vertex/texcoord
 * 3. vertex//normal
 * 4. vertex/texcoord/normal
 */
static char *parse_face_vert(char *ptr, struct facevertex *fv, int numv, int numt, int numn)
{
	if(!(ptr = parse_idx(ptr, &fv->vidx, numv)))
		return 0;
	if(*ptr != '/') return (!*ptr || isspace(*ptr)) ? ptr : 0;

	if(*++ptr == '/') {	/* no texcoord */
		fv->tidx = -1;
		++ptr;
	} else {
		if(!(ptr = parse_idx(ptr, &fv->tidx, numt)))
			return 0;
		if(*ptr != '/') return (!*ptr || isspace(*ptr)) ? ptr : 0;
		++ptr;
	}

	if(!(ptr = parse_idx(ptr, &fv->nidx, numn)))
		return 0;
	return (!*ptr || isspace(*ptr)) ? ptr : 0;
}

static int cmp_facevert(const void *ap, const void *bp)
{
	const struct facevertex *a = ap;
	const struct facevertex *b = bp;

	if(a->vidx == b->vidx) {
		if(a->tidx == b->tidx) {
			return a->nidx - b->nidx;
		}
		return a->tidx - b->tidx;
	}
	return a->vidx - b->vidx;
}

static void free_rbnode_key(struct rbnode *n, void *cls)
{
	free(n->key);
}
