#include "camera.h"

void cam_follow(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir)
{
	cgm_vec3 up;

	cgm_vcons(&up, 0, cam->height, 0);
	cgm_vrotate(&up, cam->roll, targ_dir->x, targ_dir->y, targ_dir->z);

	cam->pos = cam->targ = *targ_pos;
	cgm_vadd_scaled(&cam->pos, targ_dir, -cam->dist);
	cgm_vadd(&cam->pos, &up);

	cgm_minv_lookat(cam->matrix, &cam->pos, &cam->targ, &up);
}

void cam_follow_step(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir, float delta)
{
	cgm_vec3 targ;

	cgm_vlerp(&targ, &cam->targ, targ_pos, delta);
	cam_follow(cam, &targ, targ_dir);
}
