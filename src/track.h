#ifndef TRACK_H_
#define TRACK_H_

#include "curve.h"
#include "3dgfx/3dgfx.h"
#include "3dgfx/mesh.h"
#include "scene.h"

struct track_segment {
	struct track *trk;

	float path_t[2];
	int path_seg;

	struct g3d_mesh mesh;
	struct scene scn;
};

struct track {
	struct curve *path;

	struct track_segment *tseg;
	int num_tseg;
};


int load_track(struct track *trk, const char *fname);
void destroy_track(struct track *trk);

int gen_track_mesh(struct track *trk, int subdiv, float twist);
int dump_track_mesh(struct track *trk, const char *fname);
int gen_track_seg_mesh(struct track *trk, int segidx, int subdiv, float twist);

float eval_track_roll(struct track *trk, float t);

#endif	/* TRACK_H_ */
