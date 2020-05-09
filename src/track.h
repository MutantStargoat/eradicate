#ifndef TRACK_H_
#define TRACK_H_

#include "curve.h"
#include "3dgfx/3dgfx.h"
#include "3dgfx/mesh.h"
#include "scene.h"
#include "resman.h"

#define NUM_TSEG_SCENE_LAYERS	4

struct track_segment {
	struct track *trk;

	float path_t[2];
	int path_seg;

	struct g3d_mesh mesh;
	struct scene scn[NUM_TSEG_SCENE_LAYERS];
};

struct track {
	struct curve *path;

	struct track_segment *tseg;
	int num_tseg;

	float finish_pos, half_pos;
	float start_pos;

	void *imgset;
};


int load_track(struct track *trk, const char *fname);
void destroy_track(struct track *trk);

int gen_track_mesh(struct track *trk, int subdiv, float twist);
int dump_track_mesh(struct track *trk, const char *fname);
int gen_track_seg_mesh(struct track *trk, int segidx, int subdiv, float twist);

float eval_track_roll(struct track *trk, float t);

#endif	/* TRACK_H_ */
