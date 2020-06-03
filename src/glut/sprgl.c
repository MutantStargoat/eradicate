#ifdef BUILD_OPENGL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include "sprite.h"
#include "gfxutil.h"
#include "game.h"

struct glsprites {
	struct sprites *ss;
	unsigned int tex;
	float *uvoffs;
	float usz, vsz;
};

#define MAX_SPRITES	32
static struct glsprites glspr[MAX_SPRITES];
static int num_glspr;

#define PAD		4

static void blit_sprite(struct sprite *spr, uint32_t *fb, int fbwidth)
{
	struct sprite_op *sop = spr->ops;
	int i, r, g, b, len, xoffs = 0;
	uint16_t *sptr;

	for(;;) {
		switch(sop->op) {
		case SOP_END:
			return;
		case SOP_ENDL:
			fb += fbwidth;
			xoffs = 0;
			break;
		case SOP_SKIP:
			xoffs += sop->size >> 1;
			break;
		case SOP_COPY:
			sptr = sop->data;
			len = sop->size >> 1;
			for(i=0; i<len; i++) {
				r = UNPACK_R16(*sptr);
				g = UNPACK_G16(*sptr);
				b = UNPACK_B16(*sptr);
				fb[xoffs + i] = PACK_RGB32(b, g, r) | 0xff000000;
				sptr++;
			}
			xoffs += sop->size >> 1;
			break;
		default:
			break;
		}
		sop++;
	}
}

int init_glsprites(struct sprites *ss)
{
	int tex_xsz, tex_ysz, npix, i, xoffs, yoffs;
	float du, dv;
	uint32_t *pixels, *pptr;
	struct glsprites *gls;

	if(num_glspr >= MAX_SPRITES) {
		fprintf(stderr, "sprgl: exceeded maximum spritesheet limit\n");
		return -1;
	}
	gls = glspr + num_glspr++;

	if(!(gls->uvoffs = malloc(ss->num_sprites * 2 * sizeof *gls->uvoffs))) {
		fprintf(stderr, "sprgl: failed to allocate sprite offset table (%d)\n", ss->num_sprites);
		num_glspr--;
		return -1;
	}

	npix = (ss->width + PAD) * (ss->height + PAD) * ss->num_sprites;
	tex_xsz = 1;
	while(tex_xsz * tex_xsz < npix) {
		tex_xsz <<= 1;
	}
	tex_ysz = tex_xsz * tex_xsz / 2 >= npix ? tex_xsz / 2 : tex_xsz;

	if(!(pixels = calloc(tex_xsz * tex_ysz, sizeof *pixels))) {
		fprintf(stderr, "sprgl: failed to allocate pixel buffer (%dx%d)\n", tex_xsz, tex_ysz);
		num_glspr--;
		return -1;
	}

	du = ss->width / (float)tex_xsz;
	dv = ss->height / (float)tex_ysz;

	xoffs = yoffs = 0;
	for(i=0; i<ss->num_sprites; i++) {
		gls->uvoffs[i * 2] = (float)xoffs / (float)tex_xsz;
		gls->uvoffs[i * 2 + 1] = (float)yoffs / (float)tex_ysz;

		pptr = pixels + yoffs * tex_xsz + xoffs;
		blit_sprite(ss->sprites + i, pptr, tex_xsz);
		xoffs += ss->width + PAD;
		if(xoffs >= tex_xsz - ss->width - PAD) {
			xoffs = 0;
			yoffs += ss->height + PAD;
		}
	}

	gls->ss = ss;
	gls->usz = du;
	gls->vsz = dv;

	glGenTextures(1, &gls->tex);
	glBindTexture(GL_TEXTURE_2D, gls->tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_xsz, tex_ysz, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);

	return 0;
}

void draw_sprite(void *dest, int fbpitch, struct sprites *ss, int idx)
{
	int i, x, y, offs;
	float u, v;
	struct glsprites *gls = 0;

	for(i=0; i<num_glspr; i++) {
		if(glspr[i].ss == ss) {
			gls = glspr + i;
			break;
		}
	}
	if(!gls) return;

	offs = (uint16_t*)dest - (uint16_t*)fb_pixels;
	x = offs % fb_width;
	y = offs / fb_width;

	glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT | GL_POLYGON_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, fb_width, fb_height, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, y, 0);
	glScalef(ss->width, ss->height, 1);

	glBindTexture(GL_TEXTURE_2D, gls->tex);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	u = gls->uvoffs[idx * 2];
	v = gls->uvoffs[idx * 2 + 1];

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(u, v);
	glVertex2f(0, 0);
	glTexCoord2f(u + gls->usz, v);
	glVertex2f(1, 0);
	glTexCoord2f(u + gls->usz, v + gls->vsz);
	glVertex2f(1, 1);
	glTexCoord2f(u, v + gls->vsz);
	glVertex2f(0, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();
}

#endif	/* BUILD_OPENGL */
