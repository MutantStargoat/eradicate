#ifdef BUILD_OPENGL
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "3dgfx.h"
#include "rbtree.h"

#ifndef GL_GENERATE_MIPMAP_SGIS
#define GL_GENERATE_MIPMAP_SGIS		0x8191
#endif

static float texmat[16];
static unsigned int flags;
static int polymode;
static struct rbtree *textures;

int g3d_init(void)
{
	if(!(textures = rb_create(RB_KEY_ADDR))) {
		fprintf(stderr, "failed to create texture cache\n");
		return -1;
	}
	g3d_reset();
	return 0;
}

void g3d_destroy(void)
{
	rb_free(textures);
}

void g3d_reset(void)
{
	rb_clear(textures);
	flags = 0;
	g3d_setopt(flags, G3D_ALL);
}

void g3d_framebuffer(int width, int height, void *pixels)
{
	g3d_viewport(0, 0, width, height);
}

void g3d_framebuffer_addr(void *pixels)
{
}

void g3d_viewport(int x, int y, int w, int h)
{
	/*glViewport(x, y, w, h);*/
}

static void gl_set_enable(unsigned int opt, int en)
{
	if(en) {
		glEnable(opt);
	} else {
		glDisable(opt);
	}
}

static void set_enable(unsigned int opt, int en)
{
	switch(opt) {
	case G3D_CULL_FACE:
		gl_set_enable(GL_CULL_FACE, en);
		break;
	case G3D_DEPTH_TEST:
		gl_set_enable(GL_DEPTH_TEST, en);
		break;
	case G3D_LIGHTING:
		gl_set_enable(GL_LIGHTING, en);
		break;
	case G3D_LIGHT0:
		gl_set_enable(GL_LIGHT0, en);
		break;
	case G3D_LIGHT1:
		gl_set_enable(GL_LIGHT1, en);
		break;
	case G3D_LIGHT2:
		gl_set_enable(GL_LIGHT2, en);
		break;
	case G3D_LIGHT3:
		gl_set_enable(GL_LIGHT3, en);
		break;
	case G3D_TEXTURE_2D:
		gl_set_enable(GL_TEXTURE_2D, en);
		break;
	case G3D_TEXTURE_MAT:
		glPushAttrib(GL_TRANSFORM_BIT);
		if(en) {
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(texmat);
		} else {
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
		}
		glPopAttrib();
		break;
	case G3D_ALPHA_BLEND:
		if(en) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		break;
	case G3D_ADD_BLEND:
		if(en) {
			glBlendFunc(GL_ONE, GL_ONE);
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		break;
	case G3D_TEXTURE_GEN:
		if(en) {
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		} else {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
		break;

	default:
		break;
	}

	if(en) {
		flags |= opt;
	} else {
		flags &= ~opt;
	}
}

void g3d_enable(unsigned int opt)
{
	set_enable(opt, 1);
}

void g3d_disable(unsigned int opt)
{
	set_enable(opt, 0);
}

void g3d_setopt(unsigned int opt, unsigned int mask)
{
	int i;
	unsigned int bit = 1;
	for(i=0; bit <= G3D_ADD_BLEND; i++) {
		if(mask & bit) {
			set_enable(mask & bit, opt & bit);
		}
		bit <<= 1;
	}
}

unsigned int g3d_getopt(unsigned int mask)
{
	return flags & mask;
}

void g3d_front_face(unsigned int order)
{
	glFrontFace(order == G3D_CCW ? GL_CCW : GL_CW);
}

void g3d_polygon_mode(int pmode)
{
	if(pmode == G3D_WIRE) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glShadeModel(pmode == G3D_FLAT ? GL_FLAT : GL_SMOOTH);
	}
}

int g3d_get_polygon_mode(void)
{
	return polymode;
}

static const int glmmode[] = { GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE };

void g3d_matrix_mode(int mmode)
{
	glMatrixMode(glmmode[mmode]);
}

void g3d_load_identity(void)
{
	glLoadIdentity();
}

void g3d_load_matrix(const float *m)
{
	glLoadMatrixf(m);
}

void g3d_mult_matrix(const float *m)
{
	glMultMatrixf(m);
}

void g3d_push_matrix(void)
{
	glPushMatrix();
}

void g3d_pop_matrix(void)
{
	glPopMatrix();
}

void g3d_translate(float x, float y, float z)
{
	glTranslatef(x, y, z);
}

void g3d_rotate(float angle, float x, float y, float z)
{
	glRotatef(angle, x, y, z);
}

void g3d_scale(float x, float y, float z)
{
	glScalef(x, y, z);
}

void g3d_ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
	glOrtho(left, right, bottom, top, znear, zfar);
}

void g3d_frustum(float left, float right, float bottom, float top, float znear, float zfar)
{
	glFrustum(left, right, bottom, top, znear, zfar);
}

void g3d_perspective(float vfov, float aspect, float znear, float zfar)
{
	float theta = (float)M_PI * 0.5f * vfov / 180.0f;
	float dy = tan(theta) * znear;
	float dx = dy * aspect;

	glFrustum(-dx, dx, -dy, dy, znear, zfar);
}

const float *g3d_get_matrix(int which, float *m)
{
	static float mvmat[16], projmat[16], texmat[16];

	switch(which) {
	case G3D_MODELVIEW:
		if(!m) m = mvmat;
		glGetFloatv(GL_MODELVIEW_MATRIX, m);
		break;
	case G3D_PROJECTION:
		if(!m) m = projmat;
		glGetFloatv(GL_PROJECTION_MATRIX, m);
		break;
	case G3D_TEXTURE:
		if(!m) m = texmat;
		glGetFloatv(GL_TEXTURE_MATRIX, m);
		break;
	}
	return m;
}

void g3d_light_pos(int idx, float x, float y, float z)
{
	float pos[4];
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
	pos[3] = 1.0f;
	glLightfv(GL_LIGHT0 + idx, GL_POSITION, pos);
}

void g3d_light_dir(int idx, float x, float y, float z)
{
	float dir[4];
	dir[0] = x;
	dir[1] = y;
	dir[2] = z;
	dir[3] = 0.0f;
	glLightfv(GL_LIGHT0 + idx, GL_POSITION, dir);
}

void g3d_light_color(int idx, float r, float g, float b)
{
	float col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
	glLightfv(GL_LIGHT0 + idx, GL_DIFFUSE, col);
}

void g3d_light_ambient(float r, float g, float b)
{
	float col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col);
}

void g3d_mtl_diffuse(float r, float g, float b)
{
	float col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
}

void g3d_mtl_specular(float r, float g, float b)
{
	float col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
}

void g3d_mtl_shininess(float shin)
{
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
}

void g3d_set_texture(int xsz, int ysz, void *pixels)
{
	struct rbnode *node;
	unsigned int tex;

	if((node = rb_find(textures, pixels))) {
		tex = (int)node->data;
		glBindTexture(GL_TEXTURE_2D, tex);
		return;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xsz, ysz, 0, GL_RGB,
			GL_UNSIGNED_SHORT_5_6_5, pixels);

	rb_insert(textures, pixels, (void*)tex);
}

static unsigned int glprim[] = {
	0,
	GL_POINTS,
	GL_LINES,
	GL_TRIANGLES,
	GL_QUADS
};

void g3d_draw(int prim, const struct g3d_vertex *varr, int varr_size)
{
	glVertexPointer(3, GL_FLOAT, sizeof *varr, &varr->x);
	glNormalPointer(GL_FLOAT, sizeof *varr, &varr->nx);
	glTexCoordPointer(2, GL_FLOAT, sizeof *varr, &varr->u);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof *varr, &varr->r);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(!(flags & G3D_TEXTURE_2D)) {
		glEnableClientState(GL_COLOR_ARRAY);
	}

	glDrawArrays(glprim[prim], 0, varr_size);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(!(flags & G3D_TEXTURE_2D)) {
		glDisableClientState(GL_COLOR_ARRAY);
	}
}

void g3d_draw_indexed(int prim, const struct g3d_vertex *varr, int varr_size,
		const uint16_t *iarr, int iarr_size)
{
	glVertexPointer(3, GL_FLOAT, sizeof *varr, &varr->x);
	glNormalPointer(GL_FLOAT, sizeof *varr, &varr->nx);
	glTexCoordPointer(2, GL_FLOAT, sizeof *varr, &varr->u);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof *varr, &varr->r);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(!(flags & G3D_TEXTURE_2D)) {
		glEnableClientState(GL_COLOR_ARRAY);
	}

	glDrawElements(glprim[prim], iarr_size, GL_UNSIGNED_SHORT, iarr);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(!(flags & G3D_TEXTURE_2D)) {
		glDisableClientState(GL_COLOR_ARRAY);
	}
}

void g3d_begin(int prim)
{
	glBegin(glprim[prim]);
}

void g3d_end(void)
{
	glEnd();
}

void g3d_vertex(float x, float y, float z)
{
	glVertex3f(x, y, z);
}

void g3d_normal(float x, float y, float z)
{
	glNormal3f(x, y, z);
}

void g3d_color3b(unsigned char r, unsigned char g, unsigned char b)
{
	glColor3ub(r, g, b);
}

void g3d_color4b(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	glColor4ub(r, g, b, a);
}

void g3d_color3f(float r, float g, float b)
{
	glColor3f(r, g, b);
}

void g3d_color4f(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}

void g3d_texcoord(float u, float v)
{
	glTexCoord2f(u, v);
}

#endif	/* BUILD_OPENGL */
