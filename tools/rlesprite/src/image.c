#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <png.h>
#include "image.h"

int alloc_image(struct image *img, int x, int y, int bpp)
{
	memset(img, 0, sizeof *img);
	img->width = x;
	img->height = y;
	img->bpp = bpp;
	img->scansz = img->pitch = x * bpp / 8;

	if(!(img->pixels = malloc(y * img->scansz))) {
		printf("failed to allocate %dx%d (%dbpp) pixel buffer\n", x, y, bpp);
		return -1;
	}

	/* just a guess, assume the user will fill the details, but set up reasonable
	 * defaults just in case...
	 */
	if(bpp <= 8) {
		img->nchan = 1;
		img->cmap_ncolors = 1 << bpp;
	} else if(bpp <= 24) {
		img->nchan = 3;
	} else {
		img->nchan = 4;
	}
	return 0;
}

int load_image(struct image *img, const char *fname)
{
	int i;
	FILE *fp;
	png_struct *png;
	png_info *info;
	int chan_bits, color_type;
	png_uint_32 xsz, ysz;
	png_color *palette;
	unsigned char **scanline;
	unsigned char *dptr;

	if(!(fp = fopen(fname, "rb"))) {
		printf("failed to open: %s: %s\n", fname, strerror(errno));
		return -1;
	}

	if(!(png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0))) {
		fclose(fp);
		return -1;
	}
	if(!(info = png_create_info_struct(png))) {
		fclose(fp);
		png_destroy_read_struct(&png, 0, 0);
		return -1;
	}
	if(setjmp(png_jmpbuf(png))) {
		fclose(fp);
		png_destroy_read_struct(&png, &info, 0);
		return -1;
	}

	png_init_io(png, fp);
	png_read_png(png, info, 0, 0);

	png_get_IHDR(png, info, &xsz, &ysz, &chan_bits, &color_type, 0, 0, 0);
	img->width = xsz;
	img->height = ysz;
	img->nchan = png_get_channels(png, info);
	img->bpp = img->nchan * chan_bits;
	img->scansz = img->pitch = xsz * img->bpp / 8;
	img->cmap_ncolors = 0;

	if(color_type == PNG_COLOR_TYPE_PALETTE) {
		png_get_PLTE(png, info, &palette, &img->cmap_ncolors);
		memcpy(img->cmap, palette, img->cmap_ncolors * sizeof *img->cmap);
	}

	if(!(img->pixels = malloc(ysz * img->scansz))) {
		perror("failed to allocate pixel buffer");
		fclose(fp);
		png_destroy_read_struct(&png, &info, 0);
		return -1;
	}
	dptr = img->pixels;

	scanline = (unsigned char**)png_get_rows(png, info);
	for(i=0; i<ysz; i++) {
		memcpy(dptr, scanline[i], img->scansz);
		dptr += img->pitch;
	}

	fclose(fp);
	png_destroy_read_struct(&png, &info, 0);
	return 0;
}

int save_image(struct image *img, const char *fname)
{
	int i, chan_bits, coltype;
	FILE *fp;
	png_struct *png;
	png_info *info;
	png_text txt;
	unsigned char **scanline = 0;
	unsigned char *pptr;

	if(!(fp = fopen(fname, "wb"))) {
		printf("save_image: failed to open: %s: %s\n", fname, strerror(errno));
		return -1;
	}

	if(!(png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0))) {
		fclose(fp);
		return -1;
	}
	if(!(info = png_create_info_struct(png))) {
		png_destroy_write_struct(&png, 0);
		fclose(fp);
		return -1;
	}

	txt.compression = PNG_TEXT_COMPRESSION_NONE;
	txt.key = "Software";
	txt.text = "img2tiles";
	txt.text_length = 0;

	if(setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		free(scanline);
		fclose(fp);
		return -1;
	}

	switch(img->nchan) {
	case 1:
		if(img->cmap_ncolors > 0) {
			coltype = PNG_COLOR_TYPE_PALETTE;
		} else {
			coltype = PNG_COLOR_TYPE_GRAY;
		}
		break;
	case 2:
		coltype = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 3:
		coltype = PNG_COLOR_TYPE_RGB;
		break;
	case 4:
		coltype = PNG_COLOR_TYPE_RGB_ALPHA;
		break;
	}

	chan_bits = img->bpp / img->nchan;
	png_set_IHDR(png, info, img->width, img->height, chan_bits, coltype, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_text(png, info, &txt, 1);

	if(img->cmap_ncolors > 0) {
		png_set_PLTE(png, info, (png_color*)img->cmap, img->cmap_ncolors);
	}

	if(!(scanline = malloc(img->height * sizeof *scanline))) {
		png_destroy_write_struct(&png, &info);
		fclose(fp);
		return -1;
	}

	pptr = img->pixels;
	for(i=0; i<img->height; i++) {
		scanline[i] = pptr;
		pptr += img->pitch;
	}
	png_set_rows(png, info, scanline);

	png_init_io(png, fp);
	png_write_png(png, info, 0, 0);
	png_destroy_write_struct(&png, &info);
	free(scanline);
	fclose(fp);
	return 0;
}

int conv_image_rgb565(struct image *img16, struct image *img)
{
	int i, j;
	uint16_t *dptr;
	unsigned char *sptr;

	if(!img->cmap_ncolors && img->bpp < 24) {
		fprintf(stderr, "unsupported conversion from %dbpp to 16bpp 565\n", img->bpp);
		return -1;
	}

	img16->width = img->width;
	img16->height = img->height;
	img16->nchan = img->nchan;
	img16->bpp = 16;
	img16->scansz = img16->pitch = img->width * 2;
	if(!(img16->pixels = malloc(img16->height * img16->scansz))) {
		return -1;
	}

	sptr = img->pixels;
	dptr = (uint16_t*)img16->pixels;

	for(i=0; i<img->height; i++) {
		for(j=0; j<img->width; j++) {
			unsigned int r, g, b;
			if(img->bpp <= 8 && img->cmap_ncolors) {
				int idx = *sptr++ % img->cmap_ncolors;
				r = img->cmap[idx].r >> 3;
				g = img->cmap[idx].g >> 2;
				b = img->cmap[idx].b >> 3;
			} else if(img->nchan == 1) {
				g = (*sptr++ >> 2) & 0x3f;
				r = b = g >> 1;
			} else {
				r = (*sptr++ >> 3) & 0x1f;
				g = (*sptr++ >> 2) & 0x3f;
				b = (*sptr++ >> 3) & 0x1f;
			}
			*dptr++ = (r << 11) | (g << 5) | b;
		}
	}
	return 0;
}


int cmp_image(struct image *a, struct image *b)
{
	int i;
	unsigned char *aptr = a->pixels;
	unsigned char *bptr = b->pixels;

	if(a->width != b->width || a->height != b->height || a->bpp != b->bpp || a->nchan != b->nchan) {
		return -1;
	}

	for(i=0; i<a->height; i++) {
		if(memcmp(aptr, bptr, a->scansz) != 0) {
			return -1;
		}
		aptr += a->pitch;
		bptr += b->pitch;
	}
	return 0;
}

void blit_image(struct image *src, int sx, int sy, int w, int h, struct image *dst, int dx, int dy)
{
	int i;
	unsigned char *sptr, *dptr;

	assert(src->bpp == dst->bpp);
	assert(src->nchan == dst->nchan);

	if(sx < 0) { w += sx; sx = 0; }
	if(sy < 0) { h += sy; sy = 0; }
	if(dx < 0) { w += dx; sx -= dx; dx = 0; }
	if(dy < 0) { h += dy; sy -= dy; dy = 0; }
	if(sx + w >= src->width) w = src->width - sx;
	if(sy + h >= src->height) h = src->height - sy;
	if(dx + w >= dst->width) w = dst->width - dx;
	if(dy + h >= dst->height) h = dst->height - dy;

	if(w <= 0 || h <= 0) return;

	sptr = src->pixels + sy * src->pitch + sx * src->bpp / 8;
	dptr = dst->pixels + dy * dst->pitch + dx * dst->bpp / 8;

	for(i=0; i<h; i++) {
		memcpy(dptr, sptr, w * dst->bpp / 8);
		dptr += dst->pitch;
		sptr += src->pitch;
	}
}

void image_color_offset(struct image *img, int offs)
{
	int i, sz = img->height * img->pitch;
	unsigned char *pptr = img->pixels;

	for(i=0; i<sz; i++) {
		*pptr++ += offs;
	}
}
