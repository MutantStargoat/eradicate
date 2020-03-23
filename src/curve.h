#ifndef CURVE_H_
#define CURVE_H_

#include "cgmath.h"

enum curve_eval_mode {
	CURVE_CLAMP,
	CURVE_REPEAT,
	CURVE_EXTRAP
};

struct curve_cp {
	cgm_vec3 pos, t0, t1;
};

struct curve {
	struct curve_cp *cp;
	int num_cp;
	enum curve_eval_mode mode;
	float proj_refine_thres;
};

struct curve *load_curve(const char *fname);
void free_curve(struct curve *c);

void curve_bounds(struct curve *c, cgm_vec3 *bbmin, cgm_vec3 *bbmax);

int curve_segment(struct curve *c, float t, float *seg_t);
void eval_curve(struct curve *c, float t, cgm_vec3 *ret);
void eval_tangent(struct curve *c, float t, cgm_vec3 *ret);

/*float curve_proj_slow(struct curve *c, const cgm_vec3 *p, cgm_vec3 *res);*/

/* "projects" the point to the curve and returns the parametric distance along
 * the curve of the nearest point. It needs a previous good estimate (tguess)
 * to start from, and searches in a limited parametric interval (sinterv),
 * around it, which must be narrow enough to avoid false projections.
 */
float curve_proj_guess(struct curve *c, const cgm_vec3 *p, float tguess, float sinterv, cgm_vec3 *res);

#endif	/* CURVE_H_ */
