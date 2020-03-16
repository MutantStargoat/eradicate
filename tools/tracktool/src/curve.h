#ifndef CURVE_H_
#define CURVE_H_

#include "cgmath.h"

struct curve_cp {
	cgm_vec3 pos, t0, t1;
};

struct curve {
	struct curve_cp *cp;
	int num_cp;
};

struct curve *load_curve(const char *fname);
void free_curve(struct curve *c);

int curve_segment(struct curve *c, float t, float *seg_t);
void eval_curve(struct curve *c, float t, cgm_vec3 *ret);

#endif	/* CURVE_H_ */
