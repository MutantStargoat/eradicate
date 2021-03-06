#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "treestor.h"
#include "curve.h"
#include "util.h"

#define DEF_PROJ_REFINE_THRES	1e-3f

struct curve *load_curve(const char *fname)
{
	struct ts_node *root;
	struct curve *curve;

	if(!(root = ts_load(fname))) {
		return 0;
	}
	if(strcmp(root->name, "bezcurve") != 0) {
		fprintf(stderr, "invalid curve file %s: missing \"bezcurve\" root node\n", fname);
		ts_free_tree(root);
		return 0;
	}

	curve = read_curve(root);
	ts_free_tree(root);
	return curve;
}

struct curve *read_curve(struct ts_node *root)
{
	struct curve *curve;
	struct ts_node *node;
	float *vec;
	int max_cp = 0;

	if(!(curve = calloc(1, sizeof *curve))) {
		perror("failed to allocate curve");
		return 0;
	}
	curve->proj_refine_thres = DEF_PROJ_REFINE_THRES;

	node = root->child_list;
	while(node) {
		struct curve_cp *cpptr;
		int t0_valid = 0;

		if(strcmp(node->name, "cp") != 0) {
			goto cont;
		}

		if(!(vec = ts_get_attr_vec(node, "pos", 0))) {
			fprintf(stderr, "warning: ignoring cp[%d] without a \"pos\" attribute\n",
					curve->num_cp);
			goto cont;
		}

		if(curve->num_cp >= max_cp) {
			int newnum = max_cp ? max_cp * 2 : 8;
			void *tmp = realloc(curve->cp, newnum * sizeof *curve->cp);
			if(!tmp) {
				fprintf(stderr, "failed to ralloc curve cp array to size %d\n", max_cp);
				free(curve->cp);
				free(curve);
				return 0;
			}
			curve->cp = tmp;
			max_cp = newnum;
		}

		cpptr = curve->cp + curve->num_cp++;
		cgm_vcons(&cpptr->pos, vec[0], vec[1], vec[2]);

		if((vec = ts_get_attr_vec(node, "t0", 0))) {
			cgm_vcons(&cpptr->t0, vec[0], vec[1], vec[2]);
			t0_valid = 1;
		} else {
			cpptr->t0 = cpptr->pos;
		}
		if((vec = ts_get_attr_vec(node, "t1", 0))) {
			cgm_vcons(&cpptr->t1, vec[0], vec[1], vec[2]);
			if(!t0_valid) {
				/* no t0 but we have a t1, so calculate t0 based on t1 */
				cgm_vlerp(&cpptr->t0, &cpptr->t1, &cpptr->pos, 2.0f);
			}
		} else {
			if(t0_valid) {
				/* no t1 but we have a t0, so calculate t1 based on t0 */
				cgm_vlerp(&cpptr->t1, &cpptr->t0, &cpptr->pos, 2.0f);
			} else {
				cpptr->t1 = cpptr->pos;
			}
		}

cont:
		node = node->next;
	}

	return curve;
}

void free_curve(struct curve *c)
{
	if(c) {
		free(c->cp);
		free(c);
	}
}

void curve_bounds(struct curve *c, cgm_vec3 *bbmin, cgm_vec3 *bbmax)
{
	int i, j;
	cgm_vec3 *p;

	bbmin->x = bbmin->y = bbmin->z = FLT_MAX;
	bbmax->x = bbmax->y = bbmax->z = -FLT_MAX;

	for(i=0; i<c->num_cp; i++) {
		p = &c->cp[i].pos;
		for(j=0; j<3; j++) {
			if(p->x < bbmin->x) bbmin->x = p->x;
			if(p->x > bbmax->x) bbmax->x = p->x;
			if(p->y < bbmin->y) bbmin->y = p->y;
			if(p->y > bbmax->y) bbmax->y = p->y;
			if(p->z < bbmin->z) bbmin->z = p->z;
			if(p->z > bbmax->z) bbmax->z = p->z;
			p++;
		}
	}
}

int curve_segment(struct curve *c, float t, float *seg_t)
{
	float ft;

	if(t <= 0.0f) {
		if(seg_t) *seg_t = 0.0f;
		return 0;
	}
	if(t >= 1.0f) {
		if(seg_t) *seg_t = 1.0f;
		return c->num_cp - 1;
	}

	t *= c->num_cp - 1;
	ft = floor(t);
	if(seg_t) *seg_t = t - ft;
	return cround64(ft);
}

void eval_curve(struct curve *c, float t, cgm_vec3 *ret)
{
	float ext_t;
	int seg;
	struct curve_cp *a, *b;

	switch(c->mode) {
	case CURVE_CLAMP:
		if(t <= 0.0f) {
			*ret = c->cp[0].pos;
			return;
		}
		if(t >= 1.0f) {
			*ret = c->cp[c->num_cp - 1].pos;
			return;
		}
		seg = curve_segment(c, t, &t);
		break;

	case CURVE_REPEAT:
		t = fmod(t, 1.0f);
		if(t < 0.0f) t += 1.0f;
		seg = curve_segment(c, t, &t);
		break;

	case CURVE_EXTRAP:
		seg = curve_segment(c, t, &ext_t);
		if(t >= 0.0f && t <= 1.0f) {
			t = ext_t;
		}
		break;
	}

	a = c->cp + seg;
	b = a + 1;

	ret->x = cgm_bezier(a->pos.x, a->t1.x, b->t0.x, b->pos.x, t);
	ret->y = cgm_bezier(a->pos.y, a->t1.y, b->t0.y, b->pos.y, t);
	ret->z = cgm_bezier(a->pos.z, a->t1.z, b->t0.z, b->pos.z, t);
}

void eval_tangent(struct curve *c, float t, cgm_vec3 *ret)
{
	cgm_vec3 pos;
	eval_curve(c, t, &pos);
	eval_curve(c, t + 1e-4, ret);
	cgm_vsub(ret, &pos);
	cgm_vnormalize(ret);
}

/*
float curve_proj(struct curve *c, const cgm_vec3 *p, cgm_vec3 *res)
{
}
*/

float curve_proj_guess(struct curve *c, const cgm_vec3 *pos, float tguess, cgm_vec3 *res)
{
	float step = 0.1f / c->num_cp;
	float tp, tn, t, dsqp, dsqn, dsq;
	cgm_vec3 pp, pn, p;

	/* tiny step fwd and back to determine direction */
	tp = tguess - step * 0.25f;
	tn = tguess + step * 0.25f;

	eval_curve(c, tp, &pp);
	eval_curve(c, tn, &pn);

	dsqp = cgm_vdist_sq(pos, &pp);
	dsqn = cgm_vdist_sq(pos, &pn);

	if(dsqp < dsqn) {
		step = -step;
	}
	t = tguess;
	eval_curve(c, t, &p);
	dsq = cgm_vdist_sq(pos, &p);

	do {
		tp = t;
		pp = p;
		dsqp = dsq;

		t += step;
		eval_curve(c, t, &p);
		dsq = cgm_vdist_sq(pos, &p);
	} while(dsq < dsqp);

	tn = t;
	dsqn = dsq;

	while(fabs(tn - tp) > c->proj_refine_thres) {
		t = (tp + tn) * 0.5f;
		eval_curve(c, t, &p);
		dsq = cgm_vdist_sq(pos, &p);

		if(dsqp < dsqn) {
			tn = t;
			dsqn = dsq;
		} else {
			tp = t;
			dsqp = dsq;
		}
	}

	*res = p;
	return fmod(t, 1.0f);
}
