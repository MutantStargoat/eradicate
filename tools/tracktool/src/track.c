#include <stdio.h>
#include <stdlib.h>
#include "track.h"

int create_track(struct track *trk, struct curve *curve)
{
	int i;

	trk->path = curve;
	trk->num_tseg = curve->num_cp - 1;
	if(!(trk->tseg = calloc(trk->num_tseg, sizeof *trk->tseg))) {
		fprintf(stderr, "failed to allocate %d track segments\n", trk->num_tseg);
		return -1;
	}

	for(i=0; i<trk->num_tseg; i++) {
		struct track_segment *tseg = trk->tseg + i;

		tseg->trk = trk;
		tseg->path_t[0] = (float)i / (float)trk->num_tseg;
		tseg->path_t[1] = (float)(i + 1) / (float)trk->num_tseg;
		tseg->path_seg = i;
	}

	return 0;
}

void destroy_track(struct track *trk)
{
	int i;

	if(!trk) return;

	for(i=0; i<trk->num_tseg; i++) {
		struct track_segment *tseg = trk->tseg + i;
		free(tseg->mesh.varr);
		free(tseg->mesh.iarr);
	}

	free(trk->tseg);
}

int gen_track_mesh(struct track *trk, int subdiv, float twist)
{
	int i;
	for(i=0; i<trk->num_tseg; i++) {
		if(gen_track_seg_mesh(trk, i, subdiv, twist) == -1) {
			while(--i) {
			}
			return -1;
		}
	}
	return 0;
}

#define DELTA		1e-5
/* TODO make it per-vertex attribute */
#define ROAD_RAD	3.0f
#define WING_SZ		0.8f

#define ROAD_VERTEX(vx, vy, vz, vnx, vny, vnz, vu, vv) \
	do { \
		float len; \
		vptr->x = (vx); \
		vptr->y = (vy); \
		vptr->z = (vz); \
		vptr->u = (vu); \
		vptr->v = (vv); \
		vptr->nx = (vnx); \
		vptr->ny = (vny); \
		vptr->nz = (vny); \
		len = sqrt(vptr->nx * vptr->nx + vptr->ny * vptr->ny + vptr->nz * vptr->nz); \
		if(len != 0.0f) { \
			float s = 1.0f / len; \
			vptr->nx *= s; \
			vptr->ny *= s; \
			vptr->nz *= s; \
		} \
		vptr->r = vptr->g = vptr->b = vptr->a = 255; \
		vptr++; \
	} while(0)

int gen_track_seg_mesh(struct track *trk, int segidx, int subdiv, float twist)
{
	int i, j, nverts, nidx;
	float t, roll, vend[2];
	struct track_segment *tseg = trk->tseg + segidx;
	struct g3d_mesh *m = &tseg->mesh;
	cgm_vec3 pos, cent, fwd, right, up;
	struct g3d_vertex *varr, *vptr;
	uint16_t *iarr, *iptr;

	nverts = subdiv * 8;	/* road seg 4, plus 4 for the wings */
	nidx = subdiv * 12;		/* road seg 4, plus 4 for each of the wings */

	if(!(varr = malloc(nverts * sizeof *varr))) {
		fprintf(stderr, "failed to allocate track segment %d vertex buffer (%d verts)\n", segidx, nverts);
		return -1;
	}
	vptr = varr;
	if(!(iarr = malloc(nidx * sizeof *iarr))) {
		fprintf(stderr, "failed to allocate track segment %d index buffer (%d indices)\n", segidx, nidx);
		free(varr);
		return -1;
	}
	iptr = iarr;

	free(m->varr);
	free(m->iarr);

	m->varr = varr;
	m->iarr = iarr;
	m->vcount = nverts;
	m->icount = nidx;
	m->prim = G3D_QUADS;

	for(i=0; i<subdiv; i++) {
		vend[0] = (float)i / (float)subdiv;
		vend[1] = (float)(i + 1) / (float)subdiv;

		/* add the indices for the segment quads: left wing, right wing, road */
		nverts = vptr - varr;
		/* left wing */
		*iptr++ = nverts;
		*iptr++ = nverts + 1;
		*iptr++ = nverts + 5;
		*iptr++ = nverts + 4;
		/* right wing */
		*iptr++ = nverts + 7;
		*iptr++ = nverts + 6;
		*iptr++ = nverts + 2;
		*iptr++ = nverts + 3;
		/* road */
		*iptr++ = nverts + 1;
		*iptr++ = nverts + 2;
		*iptr++ = nverts + 6;
		*iptr++ = nverts + 5;

		for(j=0; j<2; j++) {
			t = cgm_lerp(tseg->path_t[0], tseg->path_t[1], vend[j]);

			eval_curve(trk->path, t - DELTA, &pos);
			eval_curve(trk->path, t + DELTA, &fwd);
			cgm_vsub(&fwd, &pos);
			cgm_vnormalize(&fwd);

			eval_curve(trk->path, t, &cent);

			if(fabs(fwd.y) < 0.8) {
				cgm_vcons(&up, 0, 1, 0);
			} else {
				cgm_vcons(&up, 0, 0, 1);
			}

			roll = eval_track_roll(trk, t) * twist;
			cgm_vrotate(&up, roll, fwd.x, fwd.y, fwd.z);

			cgm_vcross(&right, &fwd, &up);
			cgm_vnormalize(&right);
			cgm_vcross(&up, &right, &fwd);

			/* add row of vertices */
			pos = cent;
			cgm_vadd_scaled(&pos, &right, -ROAD_RAD - WING_SZ);
			cgm_vadd_scaled(&pos, &up, WING_SZ);
			ROAD_VERTEX(pos.x, pos.y, pos.z, 1, 1, 0, 0.0f, vend[j]);

			pos = cent;
			cgm_vadd_scaled(&pos, &right, -ROAD_RAD);
			ROAD_VERTEX(pos.x, pos.y, pos.z, 0, 1, 0, 0.25f, vend[j]);
			pos = cent;
			cgm_vadd_scaled(&pos, &right, ROAD_RAD);
			ROAD_VERTEX(pos.x, pos.y, pos.z, 0, 1, 0, 0.75f, vend[j]);

			pos = cent;
			cgm_vadd_scaled(&pos, &right, ROAD_RAD + WING_SZ);
			cgm_vadd_scaled(&pos, &up, WING_SZ);
			ROAD_VERTEX(pos.x, pos.y, pos.z, -1, 1, 0, 1.0f, vend[j]);
		}
	}

	return 0;
}


/* evaluate the how much the road should twist based on x-z plane curvature */
float eval_track_roll(struct track *trk, float t)
{
	cgm_vec3 pos, pprev, pnext, next_dir, prev_dir, cross;

	eval_curve(trk->path, t, &pos);
	eval_curve(trk->path, t + 1e-4, &pnext);
	eval_curve(trk->path, t - 1e-4, &pprev);

	cgm_vcons(&next_dir, pnext.x - pos.x, 0, pnext.z - pos.z);
	cgm_vnormalize(&next_dir);
	cgm_vcons(&prev_dir, pos.x - pprev.x, 0, pos.z - pprev.z);
	cgm_vnormalize(&prev_dir);

	cgm_vcross(&cross, &next_dir, &prev_dir);
	return cross.y;
}
