#ifndef SCENE_H_
#define SCENE_H_

#include "3dgfx/mesh.h"

enum {
	OBJFLAG_VISIBLE	= 1,
	OBJFLAG_BLEND_ALPHA = 2,
	OBJFLAG_BLEND_ADD = 4,
};
#define OBJFLAG_BLEND		(OBJFLAG_BLEND_ALPHA | OBJFLAG_BLEND_ADD)
#define OBJFLAG_DEFAULT		(OBJFLAG_VISIBLE)

struct object {
	char *name;
	struct g3d_mesh mesh;
	struct image *tex;
	float alpha;
	unsigned int flags;

	cgm_vec3 centroid;
};

struct scene {
	struct object *objects;
	int num_objects, max_objects;

	int *objorder;

	void *texset;
	int own_texset;
};

void init_scene(struct scene *scn);
void destroy_scene(struct scene *scn);

int load_scene(struct scene *scn, const char *fname);

int add_scene_object(struct scene *scn, struct object *obj);
struct object *find_scene_object(struct scene *scn, const char *name);

void zsort_scene(struct scene *scn);
void draw_scene(struct scene *scn);

#endif	/* SCENE_H_ */
