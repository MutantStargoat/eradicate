#ifndef SCENE_H_
#define SCENE_H_

#include "3dgfx/mesh.h"

struct object {
	struct g3d_mesh mesh;
	struct image *tex;
};

struct scene {
	struct object *objects;
	int num_objects, max_objects;

	struct image *textures;
	int num_textures, max_textures;
};

void init_scene(struct scene *scn);
void destroy_scene(struct scene *scn);

int load_scene(struct scene *scn, const char *fname);

int add_scene_object(struct scene *scn, struct object *obj);

#endif	/* SCENE_H_ */
