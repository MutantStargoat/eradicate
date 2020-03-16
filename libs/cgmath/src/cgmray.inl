/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 */
static inline void cgm_rcons(cgm_ray *r, float x, float y, float z, float dx, float dy, float dz)
{
	r->origin.x = x;
	r->origin.y = y;
	r->origin.z = z;
	r->dir.x = dx;
	r->dir.y = dy;
	r->dir.z = dz;
}

static inline void cgm_rmul_mr(cgm_ray *ray, const float *m)
{
	cgm_vmul_m4v3(&ray->origin, m);
	cgm_vmul_m3v3(&ray->dir, m);
}

static inline void cgm_rmul_rm(cgm_ray *ray, const float *m)
{
	cgm_vmul_v3m4(&ray->origin, m);
	cgm_vmul_v3m3(&ray->dir, m);
}

static inline void cgm_rreflect(cgm_ray *ray, const cgm_vec3 *n)
{
	cgm_vreflect(&ray->dir, n);
}

static inline void cgm_rrefract(cgm_ray *ray, const cgm_vec3 *n, float ior)
{
	cgm_vrefract(&ray->dir, n, ior);
}
