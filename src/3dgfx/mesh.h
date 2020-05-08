#ifndef MESH_H_
#define MESH_H_

#include "inttypes.h"

struct g3d_mesh {
	int prim;
	struct g3d_vertex *varr;
	uint16_t *iarr;
	int vcount, icount;
};

void free_mesh(struct g3d_mesh *mesh);
void destroy_mesh(struct g3d_mesh *mesh);

int copy_mesh(struct g3d_mesh *dest, struct g3d_mesh *src);

int load_mesh(struct g3d_mesh *mesh, const char *fname);
int save_mesh(struct g3d_mesh *mesh, const char *fname);

void zsort_mesh(struct g3d_mesh *mesh);
void draw_mesh(struct g3d_mesh *mesh);

void apply_mesh_xform(struct g3d_mesh *mesh, const float *xform);
int append_mesh(struct g3d_mesh *ma, struct g3d_mesh *mb);
int indexify_mesh(struct g3d_mesh *mesh);

void normalize_mesh_normals(struct g3d_mesh *mesh);

void calc_mesh_centroid(struct g3d_mesh *mesh, float *cent);

int gen_sphere_mesh(struct g3d_mesh *mesh, float rad, int usub, int vsub);
int gen_plane_mesh(struct g3d_mesh *mesh, float width, float height, int usub, int vsub);
int gen_cube_mesh(struct g3d_mesh *mesh, float sz, int sub);
int gen_torus_mesh(struct g3d_mesh *mesh, float rad, float ringrad, int usub, int vsub);

#endif	/* MESH_H_ */
