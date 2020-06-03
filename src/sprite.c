#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "inttypes.h"
#include "sprite.h"
#include "util.h"


#define SOP_OP(op)	((op) & 0xff)
#define SOP_LEN(op)	((op) >> 16)

#pragma pack (push, 1)
struct file_header {
	char magic[8];
	uint16_t width, height, bpp;
	uint16_t count;
} PACKED;
#pragma pack (pop)


static int read_sprite(struct sprite *spr, int pixsz, FILE *fp);


void destroy_sprites(struct sprites *ss)
{
	int i;

	for(i=0; i<ss->num_sprites; i++) {
		free(ss->sprites[i].ops);
	}
	free(ss->sprites);
}

int load_sprites(struct sprites *ss, const char *fname)
{
	int i;
	FILE *fp;
	struct file_header hdr;

	ss->sprites = 0;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "failed to load sprites from %s: %s\n", fname, strerror(errno));
		return -1;
	}
	if(fread(&hdr, sizeof hdr, 1, fp) == 0) {
		fprintf(stderr, "unexpected EOF while reading from %s\n", fname);
		goto err;
	}
	if(memcmp(hdr.magic, "RLESPRIT", sizeof hdr.magic) != 0) {
		fprintf(stderr, "invalid magic in %s\n", fname);
		goto err;
	}

#ifdef BUILD_BIGENDIAN
	ss->width = BSWAP16(hdr.width);
	ss->height = BSWAP16(hdr.height);
	ss->bpp = BSWAP16(hdr.bpp);
	ss->num_sprites = BSWAP16(hdr.count);
#else
	ss->width = hdr.width;
	ss->height = hdr.height;
	ss->bpp = hdr.bpp;
	ss->num_sprites = hdr.count;
#endif

	if(!(ss->sprites = malloc(ss->num_sprites * sizeof *ss->sprites))) {
		fprintf(stderr, "failed to allocate %d sprites for %s\n", ss->num_sprites, fname);
		goto err;
	}

	for(i=0; i<ss->num_sprites; i++) {
		if(read_sprite(ss->sprites + i, (ss->bpp + 7) / 8, fp) == -1) {
			goto err;
		}
	}

	fclose(fp);
#ifdef BUILD_OPENGL
	init_glsprites(ss);
#endif
	return 0;

err:
	free(ss->sprites);
	fclose(fp);
	return -1;
}

static int read_op(uint32_t *op, FILE *fp)
{
	if(fread(op, sizeof *op, 1, fp) == 0) {
		return -1;
	}
#ifdef BUILD_BIGENDIAN
	*op = BSWAP32(*op);
#endif
	return 0;
}

static int read_sprite(struct sprite *spr, int pixsz, FILE *fp)
{
	int i, idx, max_ops, newmax, len, bufsz;
	void *tmp;
	uint32_t op;

	spr->ops = 0;
	spr->num_ops = max_ops = 0;

	do {
		/* read the op */
		if(read_op(&op, fp) == -1) {
			free(spr->ops);
			fprintf(stderr, "unexpected EOF while trying to read sprite op\n");
			return -1;
		}

		/* realloc ops array if necessary */
		if(spr->num_ops >= max_ops) {
			newmax = max_ops ? max_ops << 1 : 16;
			if(!(tmp = realloc(spr->ops, newmax * sizeof *spr->ops))) {
				fprintf(stderr, "failed to add sprite op (newmax: %d)\n", newmax);
				goto err;
			}
			spr->ops = tmp;
			max_ops = newmax;
		}

		/* append */
		idx = spr->num_ops++;
		spr->ops[idx].op = SOP_OP(op);
		len = SOP_LEN(op);
		bufsz = len * pixsz;
		spr->ops[idx].size = bufsz;

		/* if the op was copy, we need to grab the pixel data */
		if(SOP_OP(op) == SOP_COPY) {
			if(!(tmp = malloc(bufsz))) {
				fprintf(stderr, "failed to allocate sprite pixel data (%d bytes)\n", bufsz);
				goto err;
			}
			if(fread(tmp, 1, bufsz, fp) < bufsz) {
				fprintf(stderr, "unexpected EOF while trying to read sprite data (%d pixels)\n", len);
				goto err;
			}
#ifdef BUILD_BIGENDIAN
			{
				uint16_t *pix = tmp;
				for(i=0; i<len; i++) {
					pix[i] = BSWAP16(pix[i]);
				}
			}
#endif
			spr->ops[idx].data = tmp;
		} else {
			spr->ops[idx].data = 0;
		}

	} while(SOP_OP(op) != SOP_END);

	return 0;

err:
	for(i=0; i<spr->num_ops; i++) {
		free(spr->ops[i].data);
	}
	free(spr->ops);
	return -1;
}

#ifndef BUILD_OPENGL
void draw_sprite(void *dest, int fbpitch, struct sprites *ss, int idx)
{
	struct sprite_op *sop = ss->sprites[idx].ops;
	unsigned char *fbptr = dest;
	int xoffs = 0;

	for(;;) {
		assert((xoffs & 1) == 0);
		switch(sop->op) {
		case SOP_END:
			return;
		case SOP_ENDL:
			fbptr += fbpitch;
			xoffs = 0;
			break;
		case SOP_SKIP:
			xoffs += sop->size;
			break;
		case SOP_COPY:
			memcpy(fbptr + xoffs, sop->data, sop->size);
			xoffs += sop->size;
			break;
		case SOP_FILL:
			memset16(fbptr + xoffs, (uint16_t)sop->data, sop->size >> 1);
			xoffs += sop->size;
			break;
		default:
			break;
		}
		sop++;
	}
}
#endif	/* !def BUILD_OPENGL */
