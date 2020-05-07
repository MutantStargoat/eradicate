#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "track.h"
#include "treestor.h"

int load_track(struct track *trk, const char *fname)
{
	int i;
	struct curve *curve;
	struct ts_node *root, *node;

	if(!(root = ts_load(fname))) {
		return -1;
	}
	if(strcmp(root->name, "track") != 0) {
		fprintf(stderr, "invalid track file %s: root node should be \"track\"\n", fname);
		ts_free_tree(root);
		return -1;
	}
	if(!(node = ts_lookup_node(root, "track.path"))) {
		fprintf(stderr, "invalid track file %s: path node not found\n", fname);
		ts_free_tree(root);
		return -1;
	}
	if(!(curve = read_curve(node))) {
		ts_free_tree(root);
		return -1;
	}
	curve->mode = CURVE_REPEAT;
	curve->proj_refine_thres = 1e-5;

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

	/* load detail scenes for the track segments */
	node = root->child_list;
	while(node) {
		int tidx;
		const char *scn_fname;
		struct ts_node *tnode = node;
		node = node->next;

		if(strcmp(tnode->name, "segment") != 0) {
			continue;
		}
		tidx = ts_get_attr_int(tnode, "index", -1);
		if(tidx < 0 || tidx >= trk->num_tseg) {
			fprintf(stderr, "%s: ignoring segment block with invalid index (%d)\n", fname, tidx);
			continue;
		}
		if(!(scn_fname = ts_get_attr_str(tnode, "scene", 0))) {
			continue;
		}
		if(trk->tseg[tidx].scn.num_objects) {
			fprintf(stderr, "%s: ignoring multiple blocks defining scenes for segment %d\n", fname, tidx);
			continue;
		}

		if(load_scene(&trk->tseg[tidx].scn, scn_fname) == -1) {
#ifndef NDEBUG
			return -1;
#endif
		}
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

int dump_track_mesh(struct track *trk, const char *fname)
{
	int i, j, voffs;
	FILE *fp;
	struct g3d_mesh *m;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "dump_track_mesh: failed to open %s: %s\n", fname, strerror(errno));
		return -1;
	}

	voffs = 0;
	for(i=0; i<trk->num_tseg; i++) {
		m = &trk->tseg[i].mesh;

		fprintf(fp, "o trkseg%02d\n", i);

		for(j=0; j<m->vcount; j++) {
			fprintf(fp, "v %f %f %f\n", m->varr[j].x, m->varr[j].y, m->varr[j].z);
		}
		for(j=0; j<m->vcount; j++) {
			fprintf(fp, "vn %f %f %f\n", m->varr[j].nx, m->varr[j].ny, m->varr[j].nz);
		}
		for(j=0; j<m->vcount; j++) {
			fprintf(fp, "vt %f %f\n", m->varr[j].u, m->varr[j].v);
		}
		for(j=0; j<m->icount; j++) {
			int idx = m->iarr[j] + voffs + 1;
			if(j % m->prim == 0) {
				if(j) fputc('\n', fp);
				fprintf(fp, "f");
			}
			fprintf(fp, " %d/%d/%d", idx, idx, idx);
		}
		fputc('\n', fp);
		voffs += m->vcount;
	}

	fclose(fp);
	return 0;
}

#define DELTA		1e-5
/* TODO make it per-vertex attribute */
#define ROAD_RAD	6.0f
#define WING_SZ		0.8f
#define TEX_V_SCALE	8.0f

#define ROAD_VERTEX(vx, vy, vz, vnx, vny, vnz, vu, vv) \
	do { \
		float len; \
		vptr->x = (vx); \
		vptr->y = (vy); \
		vptr->z = (vz); \
		vptr->u = (vu); \
		vptr->v = (vv) * TEX_V_SCALE; \
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

	nverts = subdiv * 10;	/* road seg 6, plus 4 for the wings */
	nidx = subdiv * 16;		/* road seg 8, plus 4 for each of the wings */

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
		vend[0] = (float)(subdiv - i - 1) / (float)subdiv;
		vend[1] = (float)(subdiv - i) / (float)subdiv;

		/* add the indices for the segment quads
		 * +--+----+----+--+  <- 5, 6, 7, 8, 9
		 * |  |    |    |  |
		 * +--+----+----+--+  <- 0, 1, 2, 3, 4
		 */
		nverts = vptr - varr;
		/* left wing */
		*iptr++ = nverts;
		*iptr++ = nverts + 1;
		*iptr++ = nverts + 6;
		*iptr++ = nverts + 5;
		/* right wing */
		*iptr++ = nverts + 9;
		*iptr++ = nverts + 8;
		*iptr++ = nverts + 3;
		*iptr++ = nverts + 4;
		/* road left */
		*iptr++ = nverts + 1;
		*iptr++ = nverts + 2;
		*iptr++ = nverts + 7;
		*iptr++ = nverts + 6;
		/* road right */
		*iptr++ = nverts + 2;
		*iptr++ = nverts + 3;
		*iptr++ = nverts + 8;
		*iptr++ = nverts + 7;

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
			cgm_vadd_scaled(&pos, &up, WING_SZ * 1.5);
			ROAD_VERTEX(pos.x, pos.y, pos.z, 1, 1, 0, 0.0f, vend[j]);

			pos = cent;
			cgm_vadd_scaled(&pos, &right, -ROAD_RAD);
			ROAD_VERTEX(pos.x, pos.y, pos.z, 0, 1, 0, 0.25f, vend[j]);

			pos = cent;
			ROAD_VERTEX(pos.x, pos.y, pos.z, 0, 1, 0, 0.99f, vend[j]);

			cgm_vadd_scaled(&pos, &right, ROAD_RAD);
			ROAD_VERTEX(pos.x, pos.y, pos.z, 0, 1, 0, 0.25f, vend[j]);

			pos = cent;
			cgm_vadd_scaled(&pos, &right, ROAD_RAD + WING_SZ);
			cgm_vadd_scaled(&pos, &up, WING_SZ * 1.5);
			ROAD_VERTEX(pos.x, pos.y, pos.z, -1, 1, 0, 0.0f, vend[j]);
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
