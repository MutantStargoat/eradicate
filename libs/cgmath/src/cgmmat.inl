/* gph-cmath - C graphics math library
 * Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is free software. Feel free to use, modify, and/or redistribute
 * it under the terms of the MIT/X11 license. See LICENSE for details.
 * If you intend to redistribute parts of the code without the LICENSE file
 * replace this paragraph with the full contents of the LICENSE file.
 */
static inline void cgm_mcopy(float *dest, const float *src)
{
	memcpy(dest, src, 16 * sizeof(float));
}

static inline void cgm_mzero(float *m)
{
	static float z[16];
	cgm_mcopy(m, z);
}

static inline void cgm_midentity(float *m)
{
	static float id[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	cgm_mcopy(m, id);
}

static inline void cgm_mmul(float *a, const float *b)
{
	int i, j;
	float res[16];
	float *resptr = res;
	float *arow = a;

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			*resptr++ = arow[0] * b[j] + arow[1] * b[4 + j] +
				arow[2] * b[8 + j] + arow[3] * b[12 + j];
		}
		arow += 4;
	}
	cgm_mcopy(a, res);
}

static inline void cgm_mpremul(float *a, const float *b)
{
	int i, j;
	float res[16];
	float *resptr = res;
	const float *brow = b;

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			*resptr++ = brow[0] * a[j] + brow[1] * a[4 + j] +
				brow[2] * a[8 + j] + brow[3] * a[12 + j];
		}
		brow += 4;
	}
	cgm_mcopy(a, res);
}

static inline void cgm_msubmatrix(float *m, int row, int col)
{
	float orig[16];
	int i, j, subi, subj;

	cgm_mcopy(orig, m);

	subi = 0;
	for(i=0; i<4; i++) {
		if(i == row) continue;

		subj = 0;
		for(j=0; j<4; j++) {
			if(j == col) continue;

			m[subi * 4 + subj++] = orig[i * 4 + j];
		}
		subi++;
	}

	cgm_mupper3(m);
}

static inline void cgm_mupper3(float *m)
{
	m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
	m[15] = 1.0f;
}

static inline float cgm_msubdet(const float *m, int row, int col)
{
	float tmp[16];
	float subdet00, subdet01, subdet02;

	cgm_mcopy(tmp, m);
	cgm_msubmatrix(tmp, row, col);

	subdet00 = tmp[5] * tmp[10] - tmp[6] * tmp[9];
	subdet01 = tmp[4] * tmp[10] - tmp[6] * tmp[8];
	subdet02 = tmp[4] * tmp[9] - tmp[5] * tmp[8];

	return tmp[0] * subdet00 - tmp[1] * subdet01 + tmp[2] * subdet02;
}

static inline float cgm_mcofactor(const float *m, int row, int col)
{
	float min = cgm_msubdet(m, row, col);
	return (row + col) & 1 ? -min : min;
}

static inline float cgm_mdet(const float *m)
{
	return m[0] * cgm_msubdet(m, 0, 0) - m[1] * cgm_msubdet(m, 0, 1) +
		m[2] * cgm_msubdet(m, 0, 2) - m[3] * cgm_msubdet(m, 0, 3);
}

static inline void cgm_mtranspose(float *m)
{
	int i, j;
	for(i=0; i<4; i++) {
		for(j=0; j<i; j++) {
			int a = i * 4 + j;
			int b = j * 4 + i;
			float tmp = m[a];
			m[a] = m[b];
			m[b] = tmp;
		}
	}
}

static inline void cgm_mcofmatrix(float *m)
{
	float tmp[16];
	int i, j;

	cgm_mcopy(tmp, m);

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			m[i * 4 + j] = cgm_mcofactor(tmp, i, j);
		}
	}
}

static inline int cgm_minverse(float *m)
{
	int i, j;
	float tmp[16];
	float inv_det;
	float det = cgm_mdet(m);
	if(det == 0.0f) return -1;
	inv_det = 1.0f / det;

	cgm_mcopy(tmp, m);

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			m[i * 4 + j] = cgm_mcofactor(tmp, j, i) * inv_det;	/* transposed */
		}
	}
	return 0;
}

static inline void cgm_mtranslation(float *m, float x, float y, float z)
{
	cgm_midentity(m);
	m[12] = x;
	m[13] = y;
	m[14] = z;
}

static inline void cgm_mscaling(float *m, float sx, float sy, float sz)
{
	cgm_mzero(m);
	m[0] = sx;
	m[5] = sy;
	m[10] = sz;
	m[15] = 1.0f;
}

static inline void cgm_mrotation_x(float *m, float angle)
{
	float sa = sin(angle);
	float ca = cos(angle);

	cgm_midentity(m);
	m[5] = ca;
	m[6] = sa;
	m[9] = -sa;
	m[10] = ca;
}

static inline void cgm_mrotation_y(float *m, float angle)
{
	float sa = sin(angle);
	float ca = cos(angle);

	cgm_midentity(m);
	m[0] = ca;
	m[2] = -sa;
	m[8] = sa;
	m[10] = ca;
}

static inline void cgm_mrotation_z(float *m, float angle)
{
	float sa = sin(angle);
	float ca = cos(angle);

	cgm_midentity(m);
	m[0] = ca;
	m[1] = sa;
	m[4] = -sa;
	m[5] = ca;
}

static inline void cgm_mrotation_axis(float *m, int idx, float angle)
{
	switch(idx) {
	case 0:
		cgm_mrotation_x(m, angle);
		break;
	case 1:
		cgm_mrotation_y(m, angle);
		break;
	case 2:
		cgm_mrotation_z(m, angle);
		break;
	}
}

static inline void cgm_mrotation(float *m, float angle, float x, float y, float z)
{
	float sa = sin(angle);
	float ca = cos(angle);
	float invca = 1.0f - ca;
	float xsq = x * x;
	float ysq = y * y;
	float zsq = z * z;

	cgm_mzero(m);
	m[15] = 1.0f;

	m[0] = xsq + (1.0f - xsq) * ca;
	m[4] = x * y * invca - z * sa;
	m[8] = x * z * invca + y * sa;

	m[1] = x * y * invca + z * sa;
	m[5] = ysq + (1.0f - ysq) * ca;
	m[9] = y * z * invca - x * sa;

	m[2] = x * z * invca - y * sa;
	m[6] = y * z * invca + x * sa;
	m[10] = zsq + (1.0f - zsq) * ca;
}

static inline void cgm_mrotation_euler(float *m, float a, float b, float c, int mode)
{
	/* this array must match the EulerMode enum */
	static const int axis[][3] = {
		{0, 1, 2}, {0, 2, 1},
		{1, 0, 2}, {1, 2, 0},
		{2, 0, 1}, {2, 1, 0},
		{2, 0, 2}, {2, 1, 2},
		{1, 0, 1}, {1, 2, 1},
		{0, 1, 0}, {0, 2, 0}
	};

	float ma[16], mb[16];
	cgm_mrotation_axis(ma, axis[mode][0], a);
	cgm_mrotation_axis(mb, axis[mode][1], b);
	cgm_mrotation_axis(m, axis[mode][2], c);
	cgm_mmul(m, mb);
	cgm_mmul(m, ma);
}

static inline void cgm_mrotation_quat(float *m, const cgm_quat *q)
{
	float xsq2 = 2.0f * q->x * q->x;
	float ysq2 = 2.0f * q->y * q->y;
	float zsq2 = 2.0f * q->z * q->z;
	float sx = 1.0f - ysq2 - zsq2;
	float sy = 1.0f - xsq2 - zsq2;
	float sz = 1.0f - xsq2 - ysq2;

	m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
	m[15] = 1.0f;

	m[0] = sx;
	m[1] = 2.0f * q->x * q->y + 2.0f * q->w * q->z;
	m[2] = 2.0f * q->z * q->x - 2.0f * q->w * q->y;
	m[4] = 2.0f * q->x * q->y - 2.0f * q->w * q->z;
	m[5] = sy;
	m[6] = 2.0f * q->y * q->z + 2.0f * q->w * q->x;
	m[8] = 2.0f * q->z * q->x + 2.0f * q->w * q->y;
	m[9] = 2.0f * q->y * q->z - 2.0f * q->w * q->x;
	m[10] = sz;
}

static inline void cgm_mtranslate(float *m, float x, float y, float z)
{
	float tm[16];
	cgm_mtranslation(tm, x, y, z);
	cgm_mmul(m, tm);
}

static inline void cgm_mscale(float *m, float sx, float sy, float sz)
{
	float sm[16];
	cgm_mscaling(sm, sx, sy, sz);
	cgm_mmul(m, sm);
}

static inline void cgm_mrotate_x(float *m, float angle)
{
	float rm[16];
	cgm_mrotation_x(rm, angle);
	cgm_mmul(m, rm);
}

static inline void cgm_mrotate_y(float *m, float angle)
{
	float rm[16];
	cgm_mrotation_y(rm, angle);
	cgm_mmul(m, rm);
}

static inline void cgm_mrotate_z(float *m, float angle)
{
	float rm[16];
	cgm_mrotation_z(rm, angle);
	cgm_mmul(m, rm);
}

static inline void cgm_mrotate_axis(float *m, int idx, float angle)
{
	float rm[16];
	cgm_mrotation_axis(rm, idx, angle);
	cgm_mmul(m, rm);
}

static inline void cgm_mrotate(float *m, float angle, float x, float y, float z)
{
	float rm[16];
	cgm_mrotation(rm, angle, x, y, z);
	cgm_mmul(m, rm);
}

static inline void cgm_mrotate_euler(float *m, float a, float b, float c, int mode)
{
	float rm[16];
	cgm_mrotation_euler(rm, a, b, c, mode);
	cgm_mmul(m, rm);
}

static inline void cgm_mrotate_quat(float *m, const cgm_quat *q)
{
	float rm[16];
	cgm_mrotation_quat(rm, q);
	cgm_mmul(m, rm);
}


static inline void cgm_mpretranslate(float *m, float x, float y, float z)
{
	float tm[16];
	cgm_mtranslation(tm, x, y, z);
	cgm_mpremul(m, tm);
}

static inline void cgm_mprescale(float *m, float sx, float sy, float sz)
{
	float sm[16];
	cgm_mscaling(sm, sx, sy, sz);
	cgm_mpremul(m, sm);
}

static inline void cgm_mprerotate_x(float *m, float angle)
{
	float rm[16];
	cgm_mrotation_x(rm, angle);
	cgm_mpremul(m, rm);
}

static inline void cgm_mprerotate_y(float *m, float angle)
{
	float rm[16];
	cgm_mrotation_y(rm, angle);
	cgm_mpremul(m, rm);
}

static inline void cgm_mprerotate_z(float *m, float angle)
{
	float rm[16];
	cgm_mrotation_z(rm, angle);
	cgm_mpremul(m, rm);
}

static inline void cgm_mprerotate_axis(float *m, int idx, float angle)
{
	float rm[16];
	cgm_mrotation_axis(rm, idx, angle);
	cgm_mpremul(m, rm);
}

static inline void cgm_mprerotate(float *m, float angle, float x, float y, float z)
{
	float rm[16];
	cgm_mrotation(rm, angle, x, y, z);
	cgm_mpremul(m, rm);
}

static inline void cgm_mprerotate_euler(float *m, float a, float b, float c, int mode)
{
	float rm[16];
	cgm_mrotation_euler(rm, a, b, c, mode);
	cgm_mpremul(m, rm);
}

static inline void cgm_mprerotate_quat(float *m, const cgm_quat *q)
{
	float rm[16];
	cgm_mrotation_quat(rm, q);
	cgm_mpremul(m, rm);
}


static inline void cgm_mget_translation(const float *m, cgm_vec3 *res)
{
	res->x = m[12];
	res->y = m[13];
	res->z = m[14];
}

/* Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
 * article "Quaternion Calculus and Fast Animation".
 * adapted from: http://www.geometrictools.com/LibMathematics/Algebra/Wm5Quaternion.inl
 */
static inline void cgm_mget_rotation(const float *m, cgm_quat *res)
{
	static const int next[3] = {1, 2, 0};
	float quat[4];
	int i, j, k;

	float trace = m[0] + m[5] + m[10];
	float root;

	if(trace > 0.0f) {
		/* |w| > 1/2 */
		root = sqrt(trace + 1.0f);	/* 2w */
		res->w = 0.5f * root;
		root = 0.5f / root;	/* 1 / 4w */
		res->x = (m[6] - m[9]) * root;
		res->y = (m[8] - m[2]) * root;
		res->z = (m[1] - m[4]) * root;
	} else {
		/* |w| <= 1/2 */
		i = 0;
		if(m[5] > m[0]) {
			i = 1;
		}
		if(m[10] > m[i * 4 + i]) {
			i = 2;
		}
		j = next[i];
		k = next[j];

		root = sqrt(m[i * 4 + i] - m[j * 4 + j] - m[k * 4 + k] + 1.0f);
		quat[i + 1] = 0.5f * root;
		root = 0.5f / root;
		quat[0] = (m[j + 4 + k] - m[k * 4 + j]) * root;
		quat[j + 1] = (m[i * 4 + j] - m[j * 4 + i]) * root;
		quat[k + 1] = (m[i * 4 + k] - m[k * 4 + i]) * root;
		res->w = quat[0];
		res->x = quat[1];
		res->y = quat[2];
		res->z = quat[3];
	}
}

static inline void cgm_mget_scaling(const float *m, cgm_vec3 *res)
{
	res->x = sqrt(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]);
	res->y = sqrt(m[1] * m[1] + m[5] * m[5] + m[9] * m[9]);
	res->z = sqrt(m[2] * m[2] + m[6] * m[6] + m[10] * m[10]);
}

static inline void cgm_mget_frustum_plane(const float *m, int p, cgm_vec4 *res)
{
	int row = p >> 1;
	const float *rowptr = m + row * 4;

	if((p & 1) == 0) {
		res->x = m[12] + rowptr[0];
		res->y = m[13] + rowptr[1];
		res->z = m[14] + rowptr[2];
		res->w = m[15] + rowptr[3];
	} else {
		res->x = m[12] - rowptr[0];
		res->y = m[13] - rowptr[1];
		res->z = m[14] - rowptr[2];
		res->w = m[15] - rowptr[3];
	}
}

static inline void cgm_mlookat(float *m, const cgm_vec3 *pos, const cgm_vec3 *targ,
		const cgm_vec3 *up)
{
	float trans[16];
	cgm_vec3 dir = *targ, right, vup;

	cgm_vsub(&dir, pos);
	cgm_vnormalize(&dir);
	cgm_vcross(&right, &dir, up);
	cgm_vnormalize(&right);
	cgm_vcross(&vup, &right, &dir);
	cgm_vnormalize(&vup);

	cgm_midentity(m);
	m[0] = right.x;
	m[1] = right.y;
	m[2] = right.z;
	m[4] = vup.x;
	m[5] = vup.y;
	m[6] = vup.z;
	m[8] = -dir.x;
	m[9] = -dir.y;
	m[10] = -dir.z;

	cgm_mtranslation(trans, pos->x, pos->y, pos->z);
	cgm_mmul(m, trans);
}

static inline void cgm_minv_lookat(float *m, const cgm_vec3 *pos, const cgm_vec3 *targ,
		const cgm_vec3 *up)
{
	float rot[16];
	cgm_vec3 dir = *targ, right, vup;

	cgm_vsub(&dir, pos);
	cgm_vnormalize(&dir);
	cgm_vcross(&right, &dir, up);
	cgm_vnormalize(&right);
	cgm_vcross(&vup, &right, &dir);
	cgm_vnormalize(&vup);

	cgm_midentity(rot);
	rot[0] = right.x;
	rot[4] = right.y;
	rot[8] = right.z;
	rot[1] = vup.x;
	rot[5] = vup.y;
	rot[9] = vup.z;
	rot[2] = -dir.x;
	rot[6] = -dir.y;
	rot[10] = -dir.z;

	cgm_mtranslation(m, -pos->x, -pos->y, -pos->z);
	cgm_mmul(m, rot);
}

static inline void cgm_mortho(float *m, float left, float right, float bot, float top,
		float znear, float zfar)
{
	float dx = right - left;
	float dy = top - bot;
	float dz = zfar - znear;

	cgm_midentity(m);
	m[0] = 2.0f / dx;
	m[5] = 2.0f / dy;
	m[10] = -2.0f / dz;
	m[12] = -(right + left) / dx;
	m[13] = -(top + bot) / dy;
	m[14] = -(zfar + znear) / dz;
}

static inline void cgm_mfrustum(float *m, float left, float right, float bot, float top,
		float znear, float zfar)
{
	float dx = right - left;
	float dy = top - bot;
	float dz = zfar - znear;

	cgm_mzero(m);
	m[0] = 2.0f * znear / dx;
	m[5] = 2.0f * znear / dy;
	m[8] = (right + left) / dx;
	m[9] = (top + bot) / dy;
	m[10] = -(zfar + znear) / dz;
	m[14] = -2.0f * zfar * znear / dz;
	m[11] = -1.0f;
}

static inline void cgm_mperspective(float *m, float vfov, float aspect, float znear, float zfar)
{
	float s = 1.0f / (float)tan(vfov / 2.0f);
	float range = znear - zfar;

	cgm_mzero(m);
	m[0] = s / aspect;
	m[5] = s;
	m[10] = (znear + zfar) / range;
	m[14] = 2.0f * znear * zfar / range;
	m[11] = -1.0f;
}

static inline void cgm_mmirror(float *m, float a, float b, float c, float d)
{
	m[0] = 1.0f - 2.0f * a * a;
	m[5] = 1.0f - 2.0f * b * b;
	m[10] = 1.0f - 2.0f * c * c;
	m[15] = 1.0f;

	m[1] = m[4] = -2.0f * a * b;
	m[2] = m[8] = -2.0f * a * c;
	m[6] = m[9] = -2.0f * b * c;

	m[12] = -2.0f * a * d;
	m[13] = -2.0f * b * d;
	m[14] = -2.0f * c * d;

	m[3] = m[7] = m[11] = 0.0f;
}
