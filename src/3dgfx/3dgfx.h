#ifndef THREEDGFX_H_
#define THREEDGFX_H_

#include "inttypes.h"
#include "gfxutil.h"

#define G3D_PIXFMT16
typedef uint16_t g3d_pixel;

#ifdef G3D_PIXFMT16
#define G3D_PACK_RGB(r, g, b)	PACK_RGB16(r, g, b)
#define G3D_UNPACK_R(c)			UNPACK_R16(c)
#define G3D_UNPACK_G(c)			UNPACK_G16(c)
#define G3D_UNPACK_B(c)			UNPACK_B16(c)
#endif
#ifdef G3D_PIXFMT32
#define G3D_PACK_RGB(r, g, b)	PACK_RGB32(r, g, b)
#define G3D_UNPACK_R(c)			UNPACK_R32(c)
#define G3D_UNPACK_G(c)			UNPACK_G32(c)
#define G3D_UNPACK_B(c)			UNPACK_B32(c)
#endif


struct g3d_vertex {
	float x, y, z, w;
	float nx, ny, nz;
	float u, v;
	unsigned char r, g, b, a;
};

enum {
	G3D_POINTS = 1,
	G3D_LINES = 2,
	G3D_TRIANGLES = 3,
	G3D_QUADS = 4
};

/* g3d_enable/g3d_disable bits */
enum {
	G3D_CULL_FACE	= 0x000001,
	G3D_DEPTH_TEST	= 0x000002,	/* XXX not implemented */
	G3D_LIGHTING	= 0x000004,
	G3D_LIGHT0		= 0x000008,
	G3D_LIGHT1		= 0x000010,
	G3D_LIGHT2		= 0x000020,
	G3D_LIGHT3		= 0x000040,
	G3D_TEXTURE_2D	= 0x000080,
	G3D_BLEND		= 0x000100,
	G3D_TEXTURE_GEN	= 0x000200,
	G3D_CLIP_FRUSTUM = 0x000800,/* when disabled, don't clip against the frustum */
	G3D_CLIP_PLANE0 = 0x001000,	/* user-defined 3D clipping planes XXX not impl. */
	G3D_CLIP_PLANE1 = 0x002000,
	G3D_CLIP_PLANE2 = 0x004000,
	G3D_CLIP_PLANE3 = 0x008000,

	G3D_TEXTURE_MAT	= 0x010000,
	G3D_SPECULAR	= 0x020000,

	G3D_ALL = 0x7fffffff
};

/* arg to g3d_front_face */
enum { G3D_CCW, G3D_CW };

/* arg to g3d_polygon_mode */
enum {
	G3D_WIRE,
	G3D_FLAT,
	G3D_GOURAUD
};

/* matrix stacks */
enum {
	G3D_MODELVIEW,
	G3D_PROJECTION,
	G3D_TEXTURE,

	G3D_NUM_MATRICES
};

int g3d_init(void);
void g3d_destroy(void);

void g3d_framebuffer(int width, int height, void *pixels);
void g3d_framebuffer_addr(void *pixels);
void g3d_viewport(int x, int y, int w, int h);

void g3d_enable(unsigned int opt);
void g3d_disable(unsigned int opt);
void g3d_setopt(unsigned int opt, unsigned int mask);
unsigned int g3d_getopt(unsigned int mask);

void g3d_front_face(unsigned int order);
void g3d_polygon_mode(int pmode);

void g3d_matrix_mode(int mmode);

void g3d_load_identity(void);
void g3d_load_matrix(const float *m);
void g3d_mult_matrix(const float *m);
void g3d_push_matrix(void);
void g3d_pop_matrix(void);

void g3d_translate(float x, float y, float z);
void g3d_rotate(float angle, float x, float y, float z);
void g3d_scale(float x, float y, float z);
void g3d_ortho(float left, float right, float bottom, float top, float znear, float zfar);
void g3d_frustum(float left, float right, float bottom, float top, float znear, float zfar);
void g3d_perspective(float vfov, float aspect, float znear, float zfar);

/* returns pointer to the *internal* matrix, and if argument m is not null,
 * also copies the internal matrix there. */
const float *g3d_get_matrix(int which, float *m);

void g3d_light_pos(int idx, float x, float y, float z);
void g3d_light_dir(int idx, float x, float y, float z);
void g3d_light_color(int idx, float r, float g, float b);

void g3d_light_ambient(float r, float g, float b);

void g3d_mtl_diffuse(float r, float g, float b);
void g3d_mtl_specular(float r, float g, float b);
void g3d_mtl_shininess(float shin);

void g3d_set_texture(int xsz, int ysz, void *pixels);

void g3d_draw(int prim, const struct g3d_vertex *varr, int varr_size);
void g3d_draw_indexed(int prim, const struct g3d_vertex *varr, int varr_size,
		const uint16_t *iarr, int iarr_size);

void g3d_begin(int prim);
void g3d_end(void);
void g3d_vertex(float x, float y, float z);
void g3d_normal(float x, float y, float z);
void g3d_color3b(unsigned char r, unsigned char g, unsigned char b);
void g3d_color4b(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void g3d_color3f(float r, float g, float b);
void g3d_color4f(float r, float g, float b, float a);
void g3d_texcoord(float u, float v);

#endif	/* THREEDGFX_H_ */
