#include "camera.h"

void cam_follow(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir, float height)
{
	cgm_vec3 up;

	cgm_vcons(&up, 0, height, 0);
	cgm_vrotate(&up, cam->roll, targ_dir->x, targ_dir->y, targ_dir->z);

	cam->pos = cam->targ = *targ_pos;
	cgm_vadd_scaled(&cam->pos, targ_dir, -cam->dist);
	cgm_vadd(&cam->pos, &up);

	cam->dir = *targ_dir;
	cam->height = cam->pos.y - cam->targ.y;

	cgm_minv_lookat(cam->matrix, &cam->pos, &cam->targ, &up);
}

void cam_follow_step(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir, float height, float delta)
{
	cgm_vec3 targ, dir;

	if(delta < 0.0f) delta = 0.0f;
	if(delta > 1.0f) delta = 1.0f;

	cgm_vlerp(&targ, &cam->targ, targ_pos, delta * 3.0f);
	cgm_vlerp(&dir, &cam->dir, targ_dir, delta);
	cam_follow(cam, &targ, &dir, cgm_lerp(cam->height, height, delta));
}
