/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 */
static inline void cgm_vcons(cgm_vec3 *v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

static inline void cgm_vadd(cgm_vec3 *a, const cgm_vec3 *b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
}

static inline void cgm_vsub(cgm_vec3 *a, const cgm_vec3 *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
}

static inline void cgm_vmul(cgm_vec3 *a, const cgm_vec3 *b)
{
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
}

static inline void cgm_vscale(cgm_vec3 *v, float s)
{
	v->x *= s;
	v->y *= s;
	v->z *= s;
}

static inline void cgm_vmul_m4v3(cgm_vec3 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[4] + v->z * m[8] + m[12];
	float y = v->x * m[1] + v->y * m[5] + v->z * m[9] + m[13];
	v->z = v->x * m[2] + v->y * m[6] + v->z * m[10] + m[14];
	v->x = x;
	v->y = y;
}

static inline void cgm_vmul_v3m4(cgm_vec3 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[1] + v->z * m[2] + m[3];
	float y = v->x * m[4] + v->y * m[5] + v->z * m[6] + m[7];
	v->z = v->x * m[8] + v->y * m[9] + v->z * m[10] + m[11];
	v->x = x;
	v->y = y;
}

static inline void cgm_vmul_m3v3(cgm_vec3 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[4] + v->z * m[8];
	float y = v->x * m[1] + v->y * m[5] + v->z * m[9];
	v->z = v->x * m[2] + v->y * m[6] + v->z * m[10];
	v->x = x;
	v->y = y;
}

static inline void cgm_vmul_v3m3(cgm_vec3 *v, const float *m)
{
	float x = v->x * m[0] + v->y * m[1] + v->z * m[2];
	float y = v->x * m[4] + v->y * m[5] + v->z * m[6];
	v->z = v->x * m[8] + v->y * m[9] + v->z * m[10];
	v->x = x;
	v->y = y;
}

static inline float cgm_vdot(const cgm_vec3 *a, const cgm_vec3 *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline void cgm_vcross(cgm_vec3 *res, const cgm_vec3 *a, const cgm_vec3 *b)
{
	res->x = a->y * b->z - a->z * b->y;
	res->y = a->z * b->x - a->x * b->z;
	res->z = a->x * b->y - a->y * b->x;
}

static inline float cgm_vlength(const cgm_vec3 *v)
{
	return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

static inline float cgm_vlength_sq(const cgm_vec3 *v)
{
	return v->x * v->x + v->y * v->y + v->z * v->z;
}

static inline float cgm_vdist(const cgm_vec3 *a, const cgm_vec3 *b)
{
	float dx = a->x - b->x;
	float dy = a->y - b->y;
	float dz = a->z - b->z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

static inline float cgm_vdist_sq(const cgm_vec3 *a, const cgm_vec3 *b)
{
	float dx = a->x - b->x;
	float dy = a->y - b->y;
	float dz = a->z - b->z;
	return dx * dx + dy * dy + dz * dz;
}

static inline void cgm_vnormalize(cgm_vec3 *v)
{
	float len = cgm_vlength(v);
	if(len != 0.0f) {
		float s = 1.0f / len;
		v->x *= s;
		v->y *= s;
		v->z *= s;
	}
}

static inline void cgm_vreflect(cgm_vec3 *v, const cgm_vec3 *n)
{
	float ndotv2 = cgm_vdot(v, n) * 2.0f;
	v->x -= n->x * ndotv2;
	v->y -= n->y * ndotv2;
	v->z -= n->z * ndotv2;
}

static inline void cgm_vrefract(cgm_vec3 *v, const cgm_vec3 *n, float ior)
{
	float ndotv = cgm_vdot(v, n);
	float k = 1.0f - ior * ior * (1.0f - ndotv * ndotv);
	if(k < 0.0f) {
		cgm_vreflect(v, n);	/* TIR */
	} else {
		float sqrt_k = sqrt(k);
		v->x = ior * v->x - (ior * ndotv + sqrt_k) * n->x;
		v->y = ior * v->y - (ior * ndotv + sqrt_k) * n->y;
		v->z = ior * v->z - (ior * ndotv + sqrt_k) * n->z;
	}
}

static inline void cgm_vrotate_quat(cgm_vec3 *v, const cgm_quat *q)
{
	cgm_quat vq, inv_q = *q, tmp_q = *q;

	cgm_qcons(&vq, v->x, v->y, v->z, 0.0f);
	cgm_qinvert(&inv_q);
	cgm_qmul(&tmp_q, &vq);
	cgm_qmul(&tmp_q, &inv_q);
	cgm_vcons(v, tmp_q.x, tmp_q.y, tmp_q.z);
}

static inline void cgm_vrotate_axis(cgm_vec3 *v, int axis, float angle)
{
	float m[16];
	cgm_mrotation_axis(m, axis, angle);
	cgm_vmul_m3v3(v, m);
}

static inline void cgm_vrotate(cgm_vec3 *v, float angle, float x, float y, float z)
{
	float m[16];
	cgm_mrotation(m, angle, x, y, z);
	cgm_vmul_m3v3(v, m);
}

static inline void cgm_vrotate_euler(cgm_vec3 *v, float a, float b, float c, enum cgm_euler_mode mode)
{
	float m[16];
	cgm_mrotation_euler(m, a, b, c, mode);
	cgm_vmul_m3v3(v, m);
}

static inline void cgm_vlerp(cgm_vec3 *res, const cgm_vec3 *a, const cgm_vec3 *b, float t)
{
	res->x = a->x + (b->x - a->x) * t;
	res->y = a->y + (b->y - a->y) * t;
	res->z = a->z + (b->z - a->z) * t;
}
