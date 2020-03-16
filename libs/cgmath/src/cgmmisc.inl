/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 */
#include <stdlib.h>

static inline float cgm_deg_to_rad(float deg)
{
	return M_PI * deg / 180.0f;
}

static inline float cgm_rad_to_deg(float rad)
{
	return 180.0f * rad / M_PI;
}

static inline float cgm_smoothstep(float a, float b, float x)
{
	if(x < a) return 0.0f;
	if(x >= b) return 1.0f;

	x = (x - a) / (b - a);
	return x * x * (3.0f - 2.0f * x);
}

static inline float cgm_lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

static inline float cgm_bezier(float a, float b, float c, float d, float t)
{
	float omt, omt3, t3, f;
	t3 = t * t * t;
	omt = 1.0f - t;
	omt3 = omt * omt * omt;
	f = 3.0f * t * omt;

	return (a * omt3) + (b * f * omt) + (c * f * t) + (d * t3);
}

static inline float cgm_bspline(float a, float b, float c, float d, float t)
{
	static const float mat[] = {
		-1, 3, -3, 1,
		3, -6, 0, 4,
		-3, 3, 3, 1,
		1, 0, 0, 0
	};
	cgm_vec4 tmp, qfact;
	float tsq = t * t;

	cgm_wcons(&qfact, tsq * t, tsq, t, 1.0f);
	cgm_wcons(&tmp, a, b, c, d);
	cgm_wmul_m4v4(&tmp, mat);
	cgm_wscale(&tmp, 1.0f / 6.0f);
	return cgm_wdot(&tmp, &qfact);
}

static inline float cgm_spline(float a, float b, float c, float d, float t)
{
	static const float mat[] = {
		-1, 2, -1, 0,
		3, -5, 0, 2,
		-3, 4, 1, 0,
		1, -1, 0, 0
	};
	cgm_vec4 tmp, qfact;
	float tsq = t * t;

	cgm_wcons(&qfact, tsq * t, tsq, t, 1.0f);
	cgm_wcons(&tmp, a, b, c, d);
	cgm_wmul_m4v4(&tmp, mat);
	cgm_wscale(&tmp, 1.0f / 6.0f);
	return cgm_wdot(&tmp, &qfact);
}

static inline void cgm_discrand(cgm_vec3 *pt, float rad)
{
	float theta = 2.0f * M_PI * (float)rand() / RAND_MAX;
	float r = sqrt((float)rand() / RAND_MAX) * rad;
	pt->x = cos(theta) * r;
	pt->y = sin(theta) * r;
	pt->z = 0.0f;
}

static inline void cgm_sphrand(cgm_vec3 *pt, float rad)
{
	float u, v, theta, phi;

	u = (float)rand() / RAND_MAX;
	v = (float)rand() / RAND_MAX;

	theta = 2.0f * M_PI * u;
	phi = acos(2.0f * v - 1.0f);

	pt->x = cos(theta) * sin(phi) * rad;
	pt->y = sin(theta) * sin(phi) * rad;
	pt->z = cos(phi) * rad;
}

static inline void cgm_unproject(cgm_vec3 *res, const cgm_vec3 *norm_scrpos,
		const float *inv_viewproj)
{
	cgm_vec4 pos;

	pos.x = 2.0f * norm_scrpos->x - 1.0f;
	pos.y = 2.0f * norm_scrpos->y - 1.0f;
	pos.z = 2.0f * norm_scrpos->z - 1.0f;
	pos.w = 1.0f;

	cgm_wmul_m4v4(&pos, inv_viewproj);

	res->x = pos.x / pos.w;
	res->y = pos.y / pos.w;
	res->z = pos.z / pos.w;
}

static inline void cgm_glu_unproject(float winx, float winy, float winz,
		const float *view, const float *proj, const int *vp,
		float *objx, float *objy, float *objz)
{
	cgm_vec3 npos, res;
	float inv_pv[16];

	cgm_mcopy(inv_pv, proj);
	cgm_mmul(inv_pv, view);

	npos.x = (winx - vp[0]) / vp[2];
	npos.y = (winy - vp[1]) / vp[4];
	npos.z = winz;

	cgm_unproject(&res, &npos, inv_pv);

	*objx = res.x;
	*objy = res.y;
	*objz = res.z;
}

static inline void cgm_pick_ray(cgm_ray *ray, float nx, float ny,
		const float *viewmat, const float *projmat)
{
	cgm_vec3 npos, farpt;
	float inv_pv[16];

	cgm_mcopy(inv_pv, projmat);
	cgm_mmul(inv_pv, viewmat);

	cgm_vcons(&npos, nx, ny, 0.0f);
	cgm_unproject(&ray->origin, &npos, inv_pv);
	npos.z = 1.0f;
	cgm_unproject(&farpt, &npos, inv_pv);

	ray->dir.x = farpt.x - ray->origin.x;
	ray->dir.y = farpt.y - ray->origin.y;
	ray->dir.z = farpt.z - ray->origin.z;
}

static inline void cgm_raypos(cgm_vec3 *p, const cgm_ray *ray, float t)
{
	p->x = ray->origin.x + ray->dir.x * t;
	p->y = ray->origin.y + ray->dir.y * t;
	p->z = ray->origin.z + ray->dir.z * t;
}

static inline void cgm_bary(cgm_vec3 *bary, const cgm_vec3 *a,
		const cgm_vec3 *b, const cgm_vec3 *c, const cgm_vec3 *pt)
{
	float d00, d01, d11, d20, d21, denom;
	cgm_vec3 v0 = *b, v1 = *c, v2 = *pt;

	cgm_vsub(&v0, a);
	cgm_vsub(&v1, a);
	cgm_vsub(&v2, a);

	d00 = cgm_vdot(&v0, &v0);
	d01 = cgm_vdot(&v0, &v1);
	d11 = cgm_vdot(&v1, &v1);
	d20 = cgm_vdot(&v2, &v0);
	d21 = cgm_vdot(&v2, &v1);
	denom = d00 * d11 - d01 * d01;

	bary->y = (d11 * d20 - d01 * d21) / denom;
	bary->z = (d00 * d21 - d01 * d20) / denom;
	bary->x = 1.0f - bary->y - bary->z;
}
