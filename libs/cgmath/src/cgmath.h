/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 *
 * Function prefixes signify the data type of their operand(s):
 * - cgm_v... functions are operations on cgm_vec3 vectors
 * - cgm_w... functions are operations on cgm_vec4 vectors
 * - cgm_q... functions are operations on cgm_quat quaternions (w + xi + yj + zk)
 * - cgm_m... functions are operations on 4x4 matrices (stored as linear 16 float arrays)
 * - cgm_r... functions are operations on cgm_ray rays
 *
 * NOTE: *ALL* matrix arguments are pointers to 16 floats. Even the functions
 * which operate on 3x3 matrices, actually use the upper 3x3 of a 4x4 matrix,
 * and still expect an array of 16 floats.
 *
 * NOTE: matrices are treated by all operations as column-major, to match OpenGL
 * conventions, so everything is pretty much transposed.
*/
#ifndef CGMATH_H_
#define CGMATH_H_

#include <math.h>
#include <string.h>

typedef struct {
	float x, y;
} cgm_vec2;

typedef struct {
	float x, y, z;
} cgm_vec3;

typedef struct {
	float x, y, z, w;
} cgm_vec4, cgm_quat;

typedef struct {
	cgm_vec3 origin, dir;
} cgm_ray;

typedef enum cgm_euler_mode {
	CGM_EULER_XYZ,
	CGM_EULER_XZY,
	CGM_EULER_YXZ,
	CGM_EULER_YZX,
	CGM_EULER_ZXY,
	CGM_EULER_ZYX,
	CGM_EULER_ZXZ,
	CGM_EULER_ZYZ,
	CGM_EULER_YXY,
	CGM_EULER_YZY,
	CGM_EULER_XYX,
	CGM_EULER_XZX
} cgm_euler_mode;

#ifdef __cplusplus
extern "C" {
#endif

/* --- operations on cgm_vec3 --- */
static inline void cgm_vcons(cgm_vec3 *v, float x, float y, float z);

static inline void cgm_vadd(cgm_vec3 *a, const cgm_vec3 *b);
static inline void cgm_vsub(cgm_vec3 *a, const cgm_vec3 *b);
static inline void cgm_vmul(cgm_vec3 *a, const cgm_vec3 *b);
static inline void cgm_vscale(cgm_vec3 *v, float s);
static inline void cgm_vmul_m4v3(cgm_vec3 *v, const float *m);	/* m4x4 * v */
static inline void cgm_vmul_v3m4(cgm_vec3 *v, const float *m);	/* v * m4x4 */
static inline void cgm_vmul_m3v3(cgm_vec3 *v, const float *m);	/* m3x3 * v (m still 16 floats) */
static inline void cgm_vmul_v3m3(cgm_vec3 *v, const float *m);	/* v * m3x3 (m still 16 floats) */

static inline float cgm_vdot(const cgm_vec3 *a, const cgm_vec3 *b);
static inline void cgm_vcross(cgm_vec3 *res, const cgm_vec3 *a, const cgm_vec3 *b);
static inline float cgm_vlength(const cgm_vec3 *v);
static inline float cgm_vlength_sq(const cgm_vec3 *v);
static inline float cgm_vdist(const cgm_vec3 *a, const cgm_vec3 *b);
static inline float cgm_vdist_sq(const cgm_vec3 *a, const cgm_vec3 *b);
static inline void cgm_vnormalize(cgm_vec3 *v);

static inline void cgm_vreflect(cgm_vec3 *v, const cgm_vec3 *n);
static inline void cgm_vrefract(cgm_vec3 *v, const cgm_vec3 *n, float ior);

static inline void cgm_vrotate_quat(cgm_vec3 *v, const cgm_quat *q);
static inline void cgm_vrotate_axis(cgm_vec3 *v, int axis, float angle);
static inline void cgm_vrotate(cgm_vec3 *v, float angle, float x, float y, float z);
static inline void cgm_vrotate_euler(cgm_vec3 *v, float a, float b, float c, enum cgm_euler_mode mode);

static inline void cgm_vlerp(cgm_vec3 *res, const cgm_vec3 *a, const cgm_vec3 *b, float t);

#define cgm_velem(vptr, idx)	((&(vptr)->x)[idx])

/* --- operations on cgm_vec4 --- */
static inline void cgm_wcons(cgm_vec4 *v, float x, float y, float z, float w);

static inline void cgm_wadd(cgm_vec4 *a, const cgm_vec4 *b);
static inline void cgm_wsub(cgm_vec4 *a, const cgm_vec4 *b);
static inline void cgm_wmul(cgm_vec4 *a, const cgm_vec4 *b);
static inline void cgm_wscale(cgm_vec4 *v, float s);

static inline void cgm_wmul_m4v4(cgm_vec4 *v, const float *m);
static inline void cgm_wmul_v4m4(cgm_vec4 *v, const float *m);
static inline void cgm_wmul_m34v4(cgm_vec4 *v, const float *m);	/* doesn't affect w */
static inline void cgm_wmul_v4m43(cgm_vec4 *v, const float *m);	/* doesn't affect w */
static inline void cgm_wmul_m3v4(cgm_vec4 *v, const float *m); /* (m still 16 floats) */
static inline void cgm_wmul_v4m3(cgm_vec4 *v, const float *m); /* (m still 16 floats) */

static inline float cgm_wdot(const cgm_vec4 *a, const cgm_vec4 *b);

static inline float cgm_wlength(const cgm_vec4 *v);
static inline float cgm_wlength_sq(const cgm_vec4 *v);
static inline float cgm_wdist(const cgm_vec4 *a, const cgm_vec4 *b);
static inline float cgm_wdist_sq(const cgm_vec4 *a, const cgm_vec4 *b);
static inline void cgm_wnormalize(cgm_vec4 *v);

static inline void cgm_wlerp(cgm_vec4 *res, const cgm_vec4 *a, const cgm_vec4 *b, float t);

#define cgm_welem(vptr, idx)	((&(vptr)->x)[idx])

/* --- operations on quaternions --- */
static inline void cgm_qcons(cgm_quat *q, float x, float y, float z, float w);

static inline void cgm_qneg(cgm_quat *q);
static inline void cgm_qadd(cgm_quat *a, const cgm_quat *b);
static inline void cgm_qsub(cgm_quat *a, const cgm_quat *b);
static inline void cgm_qmul(cgm_quat *a, const cgm_quat *b);

static inline float cgm_qlength(const cgm_quat *q);
static inline float cgm_qlength_sq(const cgm_quat *q);
static inline void cgm_qnormalize(cgm_quat *q);
static inline void cgm_qconjugate(cgm_quat *q);
static inline void cgm_qinvert(cgm_quat *q);

static inline void cgm_qrotation(cgm_quat *q, float angle, float x, float y, float z);
static inline void cgm_qrotate(cgm_quat *q, float angle, float x, float y, float z);

static inline void cgm_qslerp(cgm_quat *res, const cgm_quat *a, const cgm_quat *b, float t);
static inline void cgm_qlerp(cgm_quat *res, const cgm_quat *a, const cgm_quat *b, float t);

#define cgm_qelem(qptr, idx)	((&(qptr)->x)[idx])

/* --- operations on matrices --- */
static inline void cgm_mcopy(float *dest, const float *src);
static inline void cgm_mzero(float *m);
static inline void cgm_midentity(float *m);

static inline void cgm_mmul(float *a, const float *b);
static inline void cgm_mpremul(float *a, const float *b);

static inline void cgm_msubmatrix(float *m, int row, int col);
static inline void cgm_mupper3(float *m);
static inline float cgm_msubdet(const float *m, int row, int col);
static inline float cgm_mcofactor(const float *m, int row, int col);
static inline float cgm_mdet(const float *m);
static inline void cgm_mtranspose(float *m);
static inline void cgm_mcofmatrix(float *m);
static inline int cgm_minverse(float *m);	/* returns 0 on success, -1 for singular */

static inline void cgm_mtranslation(float *m, float x, float y, float z);
static inline void cgm_mscaling(float *m, float sx, float sy, float sz);
static inline void cgm_mrotation_x(float *m, float angle);
static inline void cgm_mrotation_y(float *m, float angle);
static inline void cgm_mrotation_z(float *m, float angle);
static inline void cgm_mrotation_axis(float *m, int idx, float angle);
static inline void cgm_mrotation(float *m, float angle, float x, float y, float z);
static inline void cgm_mrotation_euler(float *m, float a, float b, float c, int mode);
static inline void cgm_mrotation_quat(float *m, const cgm_quat *q);

static inline void cgm_mtranslate(float *m, float x, float y, float z);
static inline void cgm_mscale(float *m, float sx, float sy, float sz);
static inline void cgm_mrotate_x(float *m, float angle);
static inline void cgm_mrotate_y(float *m, float angle);
static inline void cgm_mrotate_z(float *m, float angle);
static inline void cgm_mrotate_axis(float *m, int idx, float angle);
static inline void cgm_mrotate(float *m, float angle, float x, float y, float z);
static inline void cgm_mrotate_euler(float *m, float a, float b, float c, int mode);
static inline void cgm_mrotate_quat(float *m, const cgm_quat *q);

static inline void cgm_mpretranslate(float *m, float x, float y, float z);
static inline void cgm_mprescale(float *m, float sx, float sy, float sz);
static inline void cgm_mprerotate_x(float *m, float angle);
static inline void cgm_mprerotate_y(float *m, float angle);
static inline void cgm_mprerotate_z(float *m, float angle);
static inline void cgm_mprerotate_axis(float *m, int idx, float angle);
static inline void cgm_mprerotate(float *m, float angle, float x, float y, float z);
static inline void cgm_mprerotate_euler(float *m, float a, float b, float c, int mode);
static inline void cgm_mprerotate_quat(float *m, const cgm_quat *q);

static inline void cgm_mget_translation(const float *m, cgm_vec3 *res);
static inline void cgm_mget_rotation(const float *m, cgm_quat *res);
static inline void cgm_mget_scaling(const float *m, cgm_vec3 *res);
static inline void cgm_mget_frustum_plane(const float *m, int p, cgm_vec4 *res);

static inline void cgm_mlookat(float *m, const cgm_vec3 *pos, const cgm_vec3 *targ,
		const cgm_vec3 *up);
static inline void cgm_minv_lookat(float *m, const cgm_vec3 *pos, const cgm_vec3 *targ,
		const cgm_vec3 *up);
static inline void cgm_mortho(float *m, float left, float right, float bot, float top,
		float znear, float zfar);
static inline void cgm_mfrustum(float *m, float left, float right, float bot, float top,
		float znear, float zfar);
static inline void cgm_mperspective(float *m, float vfov, float aspect, float znear, float zfar);

static inline void cgm_mmirror(float *m, float a, float b, float c, float d);

/* --- operations on rays --- */
static inline void cgm_rcons(cgm_ray *r, float x, float y, float z, float dx, float dy, float dz);

static inline void cgm_rmul_mr(cgm_ray *ray, const float *m);	/* m4x4 * ray */
static inline void cgm_rmul_rm(cgm_ray *ray, const float *m);	/* ray * m4x4 */

static inline void cgm_rreflect(cgm_ray *ray, const cgm_vec3 *n);
static inline void cgm_rrefract(cgm_ray *ray, const cgm_vec3 *n, float ior);

/* --- miscellaneous utility functions --- */
static inline float cgm_deg_to_rad(float deg);
static inline float cgm_rad_to_deg(float rad);

static inline float cgm_smoothstep(float a, float b, float x);
static inline float cgm_lerp(float a, float b, float t);
static inline float cgm_bezier(float a, float b, float c, float d, float t);
static inline float cgm_bspline(float a, float b, float c, float d, float t);
static inline float cgm_spline(float a, float b, float c, float d, float t);

static inline void cgm_discrand(cgm_vec3 *v, float rad);
static inline void cgm_sphrand(cgm_vec3 *v, float rad);

static inline void cgm_unproject(cgm_vec3 *res, const cgm_vec3 *norm_scrpos,
		const float *inv_viewproj);
static inline void cgm_glu_unproject(float winx, float winy, float winz,
		const float *view, const float *proj, const int *vp,
		float *objx, float *objy, float *objz);

static inline void cgm_pick_ray(cgm_ray *ray, float nx, float ny,
		const float *viewmat, const float *projmat);

static inline void cgm_raypos(cgm_vec3 *p, const cgm_ray *ray, float t);

/* calculate barycentric coordinates of point pt in triangle (a, b, c) */
static inline void cgm_bary(cgm_vec3 *bary, const cgm_vec3 *a,
		const cgm_vec3 *b, const cgm_vec3 *c, const cgm_vec3 *pt);

#include "cgmvec3.inl"
#include "cgmvec4.inl"
#include "cgmquat.inl"
#include "cgmmat.inl"
#include "cgmray.inl"
#include "cgmmisc.inl"

#ifdef __cplusplus
}
#endif

#endif	/* CGMATH_H_ */
