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

#define CLAMP(x, a, b)	((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

void cam_follow_step(struct camera *cam, cgm_vec3 *targ_pos, cgm_vec3 *targ_dir, float height, float delta)
{
	cgm_vec3 targ, dir;
	float t, t_hard;

	t = CLAMP(delta, 0.0f, 1.0f);
	delta *= 3.0f;
	t_hard = CLAMP(delta, 0.0f, 1.0f);

	cgm_vlerp(&targ, &cam->targ, targ_pos, t_hard);
	cgm_vlerp(&dir, &cam->dir, targ_dir, t);
	cam_follow(cam, &targ, &dir, height);/*cgm_lerp(cam->height, height, delta));*/
}
