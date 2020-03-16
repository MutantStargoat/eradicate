/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 */
static inline void cgm_wcons(cgm_vec4 *v, float x, float y, float z, float w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

static inline void cgm_wadd(cgm_vec4 *a, const cgm_vec4 *b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
	a->w += b->w;
}

static inline void cgm_wsub(cgm_vec4 *a, const cgm_vec4 *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
	a->w -= b->w;
}

static inline void cgm_wmul(cgm_vec4 *a, const cgm_vec4 *b)
{
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
	a->w *= b->w;
}

static inline void cgm_wscale(cgm_vec4 *v, float s)
{
	v->x *= s;
	v->y *= s;
	v->z *= s;
	v->w *= s;
}

static inline void cgm_wmul_m4v4(cgm_vec4 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[4] + v->z * m[8] + v->w * m[12];
	float y = v->x * m[1] + v->y * m[5] + v->z * m[9] + v->w * m[13];
	float z = v->x * m[2] + v->y * m[6] + v->z * m[10] + v->w * m[14];
	v->w = v->x * m[3] + v->y * m[7] + v->z * m[11] + v->w * m[15];
	v->x = x;
	v->y = y;
	v->z = z;
}

static inline void cgm_wmul_v4m4(cgm_vec4 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[1] + v->z * m[2] + v->w * m[3];
	float y = v->x * m[4] + v->y * m[5] + v->z * m[6] + v->w * m[7];
	float z = v->x * m[8] + v->y * m[9] + v->z * m[10] + v->w * m[11];
	v->w = v->x * m[12] + v->y * m[13] + v->z * m[14] + v->w * m[15];
	v->x = x;
	v->y = y;
	v->z = z;
}

static inline void cgm_wmul_m34v4(cgm_vec4 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[4] + v->z * m[8] + v->w * m[12];
	float y = v->x * m[1] + v->y * m[5] + v->z * m[9] + v->w * m[13];
	v->z = v->x * m[2] + v->y * m[6] + v->z * m[10] + v->w * m[14];
	v->x = x;
	v->y = y;
}

static inline void cgm_wmul_v4m43(cgm_vec4 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[1] + v->z * m[2] + v->w * m[3];
	float y = v->x * m[4] + v->y * m[5] + v->z * m[6] + v->w * m[7];
	v->z = v->x * m[8] + v->y * m[9] + v->z * m[10] + v->w * m[11];
	v->x = x;
	v->y = y;
}

static inline void cgm_wmul_m3v4(cgm_vec4 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[4] + v->z * m[8];
	float y = v->x * m[1] + v->y * m[5] + v->z * m[9];
	v->z = v->x * m[2] + v->y * m[6] + v->z * m[10];
	v->x = x;
	v->y = y;
}

static inline void cgm_wmul_v4m3(cgm_vec4 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[1] + v->z * m[2];
	float y = v->x * m[4] + v->y * m[5] + v->z * m[6];
	v->z = v->x * m[8] + v->y * m[9] + v->z * m[10];
	v->x = x;
	v->y = y;
}

static inline float cgm_wdot(const cgm_vec4 *a, const cgm_vec4 *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

static inline float cgm_wlength(const cgm_vec4 *v)
{
	return sqrt(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);
}

static inline float cgm_wlength_sq(const cgm_vec4 *v)
{
	return v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w;
}

static inline float cgm_wdist(const cgm_vec4 *a, const cgm_vec4 *b)
{
	float dx = a->x - b->x;
	float dy = a->y - b->y;
	float dz = a->z - b->z;
	float dw = a->w - b->w;
	return sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
}

static inline float cgm_wdist_sq(const cgm_vec4 *a, const cgm_vec4 *b)
{
	float dx = a->x - b->x;
	float dy = a->y - b->y;
	float dz = a->z - b->z;
	float dw = a->w - b->w;
	return dx * dx + dy * dy + dz * dz + dw * dw;
}

static inline void cgm_wnormalize(cgm_vec4 *v)
{
	float len = cgm_wlength(v);
	if(len != 0.0f) {
		float s = 1.0f / len;
		v->x *= s;
		v->y *= s;
		v->z *= s;
		v->w *= s;
	}
}

static inline void cgm_wlerp(cgm_vec4 *res, const cgm_vec4 *a, const cgm_vec4 *b, float t)
{
	res->x = a->x + (b->x - a->x) * t;
	res->y = a->y + (b->y - a->y) * t;
	res->z = a->z + (b->z - a->z) * t;
	res->w = a->w + (b->w - a->w) * t;
}
