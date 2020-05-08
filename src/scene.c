#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "cgmath.h"
#include "3dgfx/3dgfx.h"
#include "scene.h"
#include "image.h"
#include "rbtree.h"
#include "resman.h"

#if defined(__WATCOMC__) || defined(_MSC_VER) || defined(__DJGPP__)
#include <malloc.h>
#else
#include <alloca.h>
#endif


void init_scene(struct scene *scn)
{
	memset(scn, 0, sizeof *scn);
}

void destroy_scene(struct scene *scn)
{
	if(!scn) return;
	free(scn->objects);
	if(scn->own_texset) {
		free_imgset(scn->texset);
	}
	memset(scn, 0, sizeof *scn);
}

struct facevertex {
	int vidx, tidx, nidx;
};

struct objmtl {
	char *name;
	float color[3];
	float alpha;
	char *tex;
	struct objmtl *next;
};

static struct objmtl *load_materials(const char *fname, const char *dir);
static struct objmtl *find_material(struct objmtl *list, const char *name);
static void free_materials(struct objmtl *list);
static char *clean_line(char *s);
static char *parse_face_vert(char *ptr, struct facevertex *fv, int numv, int numt, int numn);
static int cmp_facevert(const void *ap, const void *bp);
static void free_rbnode_key(struct rbnode *n, void *cls);
static struct image *get_texture(struct scene *scn, const char *name);

static char buf[256];

#define APPEND(xarr, x) \
	do { \
		if(xarr##_size >= xarr##_max) { \
			int newsz = xarr##_max ? xarr##_max * 2 : 16; \
			void *newarr = realloc(xarr, newsz * sizeof *xarr); \
			if(!newarr) { \
				fprintf(stderr, "load_scene: failed to resize " #xarr " to %d bytes\n", newsz * sizeof *xarr); \
				goto err; \
			} \
			xarr = newarr; \
			xarr##_max = newsz; \
		} \
		xarr[xarr##_size++] = x; \
	} while(0)

/* merge of different indices per attribute happens during face processing.
 *
 * A triplet of (vertex index/texcoord index/normal index) is used as the key
 * to search in a balanced binary search tree for vertex buffer index assigned
 * to the same triplet if it has been encountered before. That index is
 * appended to the index buffer.
 *
 * If a particular triplet has not been encountered before, a new vertex is
 * appended to the vertex buffer. The index of this new vertex is appended to
 * the index buffer, and also inserted into the tree for future searches.
 */
int load_scene(struct scene *scn, const char *fname)
{
	FILE *fp;
	struct rbtree *rbtree = 0;
	struct rbnode *node;
	cgm_vec3 v, *varr = 0, *vptr;
	cgm_vec3 norm, *narr = 0;
	cgm_vec2 tc, *tarr = 0;
	struct g3d_vertex gv, *gvarr = 0;
	uint16_t *iarr = 0;
	int varr_size = 0, varr_max = 0;
	int narr_size = 0, narr_max = 0;
	int tarr_size = 0, tarr_max = 0;
	int gvarr_size = 0, gvarr_max = 0;
	int iarr_size = 0, iarr_max = 0;
	int i, num, line_num = 0, result = -1, found_quad = 0;
	unsigned int idx, newidx;
	char *line, *ptr, *oname = 0;
	struct facevertex fv, *newfv;
	struct object obj;
	struct objmtl *mtl, *mtllist = 0;
	static char mtl_path[256];

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "load_scene: failed to open scene file: %s: %s\n", fname, strerror(errno));
		goto err;
	}

	if(!(rbtree = rb_create(cmp_facevert))) {
		fprintf(stderr, "load_scene: failed to create facevertex binary search tree\n");
		goto err;
	}
	rb_set_delete_func(rbtree, free_rbnode_key, 0);

	while(fgets(buf, sizeof buf, fp)) {
		++line_num;
		if(!(line = clean_line(buf))) continue;

		switch(line[0]) {
		case 'm':
			if(memcmp(line, "mtllib", 6) == 0) {
				if(!(line = clean_line(line + 6))) {
					continue;
				}
				strcpy(mtl_path, fname);
				if((ptr = strrchr(mtl_path, '/'))) {
					*ptr = 0;
				} else {
					mtl_path[0] = 0;
				}
				if((mtl = load_materials(line, mtl_path))) {
					if(mtllist) free_materials(mtllist);
					mtllist = mtl;
				}
			}
			break;

		case 'u':
			if(memcmp(line, "usemtl", 6) == 0) {
				if(!(line = clean_line(line + 6))) {
					continue;
				}
				mtl = find_material(mtllist, line);
			}
			break;

		case 'v':
			if(isspace(line[1])) {
				/* vertex */
				num = sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
				if(num < 3) {
					fprintf(stderr, "load_scene: %s:%d: invalid vertex definition: \"%s\"\n", fname, line_num, line);
					goto err;
				}
				APPEND(varr, v);

			} else if(line[1] == 't' && isspace(line[2])) {
				/* texcoord */
				if(sscanf(line + 3, "%f %f", &tc.x, &tc.y) != 2) {
					fprintf(stderr, "load_scene: %s:%d: invalid texcoord definition: \"%s\"\n", fname, line_num, line);
					goto err;
				}
				APPEND(tarr, tc);

			} else if(line[1] == 'n' && isspace(line[2])) {
				/* normal */
				if(sscanf(line + 3, "%f %f %f", &norm.x, &norm.y, &norm.z) != 3) {
					fprintf(stderr, "load_scene: %s:%d: invalid normal definition: \"%s\"\n", fname, line_num, line);
					goto err;
				}
				APPEND(narr, norm);
			}
			break;

		case 'f':
			if(isspace(line[1])) {
				/* face */
				ptr = line + 2;

				for(i=0; i<4; i++) {
					if(!(ptr = parse_face_vert(ptr, &fv, varr_size, tarr_size, narr_size))) {
						if(i < 3 || found_quad) {
							fprintf(stderr, "load_scene: %s:%d: invalid face definition: \"%s\"\n", fname, line_num, line);
							goto err;
						} else {
							break;
						}
					}

					if((node = rb_find(rbtree, &fv))) {
						idx = (unsigned int)node->data;
						APPEND(iarr, idx);
					} else {
						newidx = gvarr_size;
						vptr = varr + fv.vidx;

						gv.x = vptr->x;
						gv.y = vptr->y;
						gv.z = vptr->z;
						if(fv.nidx >= 0) {
							gv.nx = narr[fv.nidx].x;
							gv.ny = narr[fv.nidx].y;
							gv.nz = narr[fv.nidx].z;
						} else {
							gv.nx = gv.ny = gv.nz = 0;
						}
						if(fv.tidx >= 0) {
							gv.u = tarr[fv.tidx].x;
							gv.v = 1.0f - tarr[fv.tidx].y;
						} else {
							gv.u = gv.v = 0;
						}
						gv.r = gv.g = gv.b = gv.a = 255;

						APPEND(gvarr, gv);
						APPEND(iarr, newidx);

						if((newfv = malloc(sizeof *newfv))) {
							*newfv = fv;
						}
						if(!newfv || rb_insert(rbtree, newfv, (void*)newidx) == -1) {
							fprintf(stderr, "load_mesh: failed to insert facevertex into the tree\n");
							goto err;
						}
					}
				}
				if(i > 3) found_quad = 1;
			}
			break;

		case 'o':
			if(iarr_size) {
				obj.name = oname;
				obj.mesh.varr = gvarr;
				obj.mesh.iarr = iarr;
				obj.mesh.vcount = gvarr_size;
				obj.mesh.icount = iarr_size;
				obj.mesh.prim = found_quad ? 4 : 3;
				calc_mesh_centroid(&obj.mesh, &obj.centroid.x);
				obj.tex = mtl && mtl->tex ? get_texture(scn, mtl->tex) : 0;
				obj.alpha = mtl ? mtl->alpha : 1.0f;
				obj.flags = OBJFLAG_DEFAULT;
				if(obj.alpha < 1.0f) obj.flags |= OBJFLAG_BLEND_ALPHA;
				gvarr = 0;
				gvarr_size = gvarr_max = 0;
				iarr = 0;
				iarr_size = iarr_max = 0;
				found_quad = 0;
				mtl = 0;
				if(add_scene_object(scn, &obj) == -1) {
					fprintf(stderr, "load_scene: failed to add object\n");
					goto err;
				}
			}
			if((oname = malloc(strlen(line + 2) + 1))) {
				strcpy(oname, line + 2);
			}
			break;

		default:
			break;
		}
	}

	if(iarr_size) {
		obj.name = oname;
		obj.mesh.varr = gvarr;
		obj.mesh.iarr = iarr;
		obj.mesh.vcount = gvarr_size;
		obj.mesh.icount = iarr_size;
		obj.mesh.prim = found_quad ? 4 : 3;
		obj.tex = mtl && mtl->tex ? get_texture(scn, mtl->tex) : 0;
		obj.alpha = mtl ? mtl->alpha : 1.0f;
		obj.flags = OBJFLAG_DEFAULT;
		if(obj.alpha < 1.0f) obj.flags |= OBJFLAG_BLEND_ALPHA;
		gvarr = 0;
		iarr = 0;
		calc_mesh_centroid(&obj.mesh, &obj.centroid.x);
		if(add_scene_object(scn, &obj) == -1) {
			fprintf(stderr, "load_scene: failed to add object\n");
			goto err;
		}
		oname = 0;
	}

	result = 0;

	printf("loaded scene %s: %d meshes\n", fname, scn->num_objects);

err:
	if(fp) fclose(fp);
	free(oname);
	free(varr);
	free(narr);
	free(tarr);
	free(gvarr);
	free(iarr);
	rb_free(rbtree);
	free_materials(mtllist);
	return result;
}

static struct objmtl *load_materials(const char *fname, const char *dir)
{
	FILE *fp;
	struct objmtl *mtl = 0, *list = 0;
	char *line;
	char *path = alloca(strlen(fname) + strlen(dir) + 2);

	if(dir) {
		sprintf(path, "%s/%s", dir, fname);
	} else {
		strcpy(path, fname);
	}

	if(!(fp = fopen(path, "rb"))) {
		return 0;
	}

	while(fgets(buf, sizeof buf, fp)) {
		if(!(line = clean_line(buf))) {
			continue;
		}

		if(memcmp(line, "newmtl", 6) == 0) {
			if(!(line = clean_line(line + 6))) {
				continue;
			}
			if(!(mtl = malloc(sizeof *mtl))) {
				fprintf(stderr, "failed to allocate material\n");
				continue;
			}
			if(!(mtl->name = malloc(strlen(line) + 1))) {
				fprintf(stderr, "failed to allocate material name\n");
				free(mtl);
				continue;
			}
			strcpy(mtl->name, line);

			mtl->color[0] = mtl->color[1] = mtl->color[2] = 1.0f;
			mtl->tex = 0;
			mtl->alpha = 1.0f;

			mtl->next = list;
			list = mtl;

		} else if(mtl && memcmp(line, "Kd", 2) == 0) {
			sscanf(line + 3, "%f %f %f", mtl->color, mtl->color + 1, mtl->color + 2);

		} else if(mtl && memcmp(line, "map_Kd", 6) == 0) {
			if(!(line = clean_line(line + 6))) {
				continue;
			}
			if(!(mtl->tex = malloc(strlen(dir) + strlen(line) + 2))) {
				fprintf(stderr, "failed to allocate texutre name buffer\n");
				continue;
			}
			if(dir) {
				sprintf(mtl->tex, "%s/%s", dir, line);
			} else {
				strcpy(mtl->tex, line);
			}

		} else if(mtl && line[0] == 'd') {
			if(!(line = clean_line(line + 1))) {
				continue;
			}
			mtl->alpha = atof(line);
		}
	}

	fclose(fp);
	return list;
}

static struct objmtl *find_material(struct objmtl *list, const char *name)
{
	while(list) {
		if(strcmp(list->name, name) == 0) {
			return list;
		}
		list = list->next;
	}
	return 0;
}

static void free_materials(struct objmtl *list)
{
	struct objmtl *mtl;

	while(list) {
		mtl = list;
		list = list->next;
		free(mtl->tex);
		free(mtl);
	}
}

int add_scene_object(struct scene *scn, struct object *obj)
{
	int newsz;
	struct object *arr;

	if(scn->num_objects >= scn->max_objects) {
		newsz = scn->max_objects > 0 ? scn->max_objects * 2 : 16;
		if(!(arr = realloc(scn->objects, newsz * sizeof *scn->objects))) {
			return -1;
		}
		scn->objects = arr;
		scn->max_objects = newsz;
	}
	scn->objects[scn->num_objects++] = *obj;
	return 0;
}

struct object *find_scene_object(struct scene *scn, const char *name)
{
	int i;
	for(i=0; i<scn->num_objects; i++) {
		if(strcmp(scn->objects[i].name, name) == 0) {
			return scn->objects + i;
		}
	}
	return 0;
}

static struct {
	struct scene *scn;
	const float *xform;
} zsort_cls;

static int zsort_cmp(const void *aptr, const void *bptr)
{
	int aidx = *(int*)aptr;
	int bidx = *(int*)bptr;
	cgm_vec3 *ca = &zsort_cls.scn->objects[aidx].centroid;
	cgm_vec3 *cb = &zsort_cls.scn->objects[bidx].centroid;
	const float *m = zsort_cls.xform;
	float za, zb;

	za = m[2] * ca->x + m[6] * ca->y + m[10] * ca->z + m[14];
	zb = m[2] * cb->x + m[6] * cb->y + m[10] * cb->z + m[14];

	//za -= zb;
	return za < zb ? -1 : (za > zb ? 1 : 0);//*(int*)&za;
}

void zsort_scene(struct scene *scn)
{
	int i;

	if(!scn->num_objects) return;

	if(!scn->objorder) {
		if(!(scn->objorder = malloc(scn->num_objects * sizeof *scn->objorder))) {
			return;
		}
		for(i=0; i<scn->num_objects; i++) {
			scn->objorder[i] = i;
		}
	}

	zsort_cls.scn = scn;
	zsort_cls.xform = g3d_get_matrix(G3D_MODELVIEW, 0);

	qsort(scn->objorder, scn->num_objects, sizeof *scn->objorder, zsort_cmp);
}

void draw_scene(struct scene *scn)
{
	int i;
	struct object *obj;

	for(i=0; i<scn->num_objects; i++) {
		obj = scn->objects + (scn->objorder ? scn->objorder[i] : i);
		if(!(obj->flags & OBJFLAG_VISIBLE)) {
			continue;
		}
		if(obj->flags & OBJFLAG_BLEND_ALPHA) {
			g3d_enable(G3D_BLEND);
		}

		if(obj->tex) {
			g3d_enable(G3D_TEXTURE_2D);
			g3d_set_texture(obj->tex->width, obj->tex->height, obj->tex->pixels);
		} else {
			g3d_disable(G3D_TEXTURE_2D);
		}
		draw_mesh(&obj->mesh);

		if(obj->flags & OBJFLAG_BLEND_ALPHA) {
			g3d_disable(G3D_BLEND);
		}
	}
}

static char *clean_line(char *s)
{
	char *end;

	while(*s && isspace(*s)) ++s;
	if(!*s) return 0;

	end = s;
	while(*end && *end != '#') ++end;
	*end-- = 0;

	while(end > s && isspace(*end)) {
		*end-- = 0;
	}

	return *s ? s : 0;
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

static struct image *get_texture(struct scene *scn, const char *name)
{
	if(!scn->texset) {
		if(!(scn->texset = create_imgset())) {
			fprintf(stderr, "failed to create image-set for the scene\n");
			return 0;
		}
		scn->own_texset = 1;
	}

	return imgset_get(scn->texset, name);
}
