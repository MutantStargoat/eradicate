/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 */
static inline void cgm_qcons(cgm_quat *q, float x, float y, float z, float w)
{
	q->x = x;
	q->y = y;
	q->z = z;
	q->w = w;
}


static inline void cgm_qneg(cgm_quat *q)
{
	q->x = -q->x;
	q->y = -q->y;
	q->z = -q->z;
	q->w = -q->w;
}

static inline void cgm_qadd(cgm_quat *a, const cgm_quat *b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
	a->w += b->w;
}

static inline void cgm_qsub(cgm_quat *a, const cgm_quat *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
	a->w -= b->w;
}

static inline void cgm_qmul(cgm_quat *a, const cgm_quat *b)
{
	float x, y, z, dot;
	cgm_vec3 cross;

	dot = a->x * b->x + a->y * b->y + a->z * b->z;
	cgm_vcross(&cross, (cgm_vec3*)a, (cgm_vec3*)b);

	x = a->w * b->x + b->w * a->x + cross.x;
	y = a->w * b->y + b->w * a->y + cross.y;
	z = a->w * b->z + b->w * a->z + cross.z;
	a->w = a->w * b->w - dot;
	a->x = x;
	a->y = y;
	a->z = z;
}

static inline float cgm_qlength(const cgm_quat *q)
{
	return sqrt(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
}

static inline float cgm_qlength_sq(const cgm_quat *q)
{
	return q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
}

static inline void cgm_qnormalize(cgm_quat *q)
{
	float len = cgm_qlength(q);
	if(len != 0.0f) {
		float s = 1.0f / len;
		q->x *= s;
		q->y *= s;
		q->z *= s;
		q->w *= s;
	}
}

static inline void cgm_qconjugate(cgm_quat *q)
{
	q->x = -q->x;
	q->y = -q->y;
	q->z = -q->z;
}

static inline void cgm_qinvert(cgm_quat *q)
{
	float len_sq = cgm_qlength_sq(q);
	cgm_qconjugate(q);
	if(len_sq != 0.0f) {
		float s = 1.0f / len_sq;
		q->x *= s;
		q->y *= s;
		q->z *= s;
		q->w *= s;
	}
}

static inline void cgm_qrotation(cgm_quat *q, float angle, float x, float y, float z)
{
	float hangle = angle * 0.5f;
	float sin_ha = sin(hangle);
	q->w = cos(hangle);
	q->x = x * sin_ha;
	q->y = y * sin_ha;
	q->z = z * sin_ha;
}

static inline void cgm_qrotate(cgm_quat *q, float angle, float x, float y, float z)
{
	cgm_quat qrot;
	cgm_qrotation(&qrot, angle, x, y, z);
	cgm_qmul(q, &qrot);
}

static inline void cgm_qslerp(cgm_quat *res, const cgm_quat *quat1, const cgm_quat *q2, float t)
{
	float angle, dot, a, b, sin_angle;
	cgm_quat q1 = *quat1;

	dot = quat1->x * q2->x + quat1->y * q2->y + quat1->z * q2->z + quat1->w * q2->w;
	if(dot < 0.0f) {
		/* make sure we inteprolate across the shortest arc */
		cgm_qneg(&q1);
		dot = -dot;
	}

	/* clamp dot to [-1, 1] in order to avoid domain errors in acos due to
	 * floating point imprecisions
	 */
	if(dot < -1.0f) dot = -1.0f;
	if(dot > 1.0f) dot = 1.0f;
	angle = acos(dot);

	sin_angle = sin(angle);
	if(sin_angle == 0.0f) {
		/* use linear interpolation to avoid div/zero */
		a = 1.0f;
		b = t;
	} else {
		a = sin((1.0f - t) * angle) / sin_angle;
		b = sin(t * angle) / sin_angle;
	}

	res->x = q1.x * a + q2->x * b;
	res->y = q1.y * a + q2->y * b;
	res->z = q1.z * a + q2->z * b;
	res->w = q1.w * a + q2->w * b;
}

static inline void cgm_qlerp(cgm_quat *res, const cgm_quat *a, const cgm_quat *b, float t)
{
	res->x = a->x + (b->x - a->x) * t;
	res->y = a->y + (b->y - a->y) * t;
	res->z = a->z + (b->z - a->z) * t;
	res->w = a->w + (b->w - a->w) * t;
}
