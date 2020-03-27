#ifndef CAMERA_H_
#define CAMERA_H_

#include "cgmath.h"

struct camera {
	cgm_vec3 pos, targ;
	cgm_vec3 dir;

	float dist, height;
	float roll;

	float matrix[16];
};

void cam_follow(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir, float height);
void cam_follow_step(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir, float height, float delta);

#endif	/* CAMERA_H_ */
