#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <imago2.h>
#include "image.h"

int load_image(struct image *img, const char *fname)
{
	int len = strlen(fname);
	if(memcmp(fname + len - 4, ".565", 4) == 0) {
		FILE *fp;
		char sig[8];
		uint16_t width, height;

		if(!(fp = fopen(fname, "rb"))) {
			fprintf(stderr, "load_image: failed to open file: %s: %s\n", fname, strerror(errno));
			return -1;
		}
		if(fread(sig, 1, 8, fp) < 8) {
			fprintf(stderr, "unexpected EOF while reading: %s\n", fname);
			fclose(fp);
			return -1;
		}
		if(memcmp(sig, "IDUMP565", 8) != 0) {
			fclose(fp);
			goto not565;
		}

		if(!fread(&width, 2, 1, fp) || !fread(&height, 2, 1, fp)) {
			fprintf(stderr, "unexpected EOF while reading: %s\n", fname);
			fclose(fp);
			return -1;
		}

		if(!(img->pixels = malloc(width * height * 2))) {
			fprintf(stderr, "failed to allocate %dx%d pixel buffer for %s\n", width, height, fname);
			fclose(fp);
			return -1;
		}
		if(fread(img->pixels, 2, width * height, fp) < width * height) {
			fprintf(stderr, "unexpected EOF while reading: %s\n", fname);
			free(img->pixels);
			img->pixels = 0;
			fclose(fp);
			return -1;
		}
		fclose(fp);
		img->width = width;
		img->height = height;
		return 0;
	}
not565:

	if(!(img->pixels = img_load_pixels(fname, &img->width, &img->height, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load image: %s\n", fname);
		return -1;
	}
	return 0;
}

int dump_image(struct image *img, const char *fname)
{
	FILE *fp;
	uint16_t width, height;

	width = img->width;
	height = img->height;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "dump_image: failed to open %s: %s\n", fname, strerror(errno));
		return -1;
	}
	fwrite("IDUMP565", 1, 8, fp);
	fwrite(&width, 2, 1, fp);
	fwrite(&height, 2, 1, fp);
	fwrite(img->pixels, 2, img->width * img->height, fp);
	fclose(fp);
	return 0;
}

void destroy_image(struct image *img)
{
	if(img) {
		img_free_pixels(img->pixels);
		img->pixels = 0;
	}
}

int load_cubemap(struct image *cube, const char *fname_fmt)
{
	int i;
	char dirstr[3] = {0};
	char fname[256];

	for(i=0; i<6; i++) {
		dirstr[0] = i & 1 ? 'n' : 'p';
		dirstr[1] = i < 2 ? 'x' : (i < 4 ? 'y' : 'z');
		sprintf(fname, fname_fmt, dirstr);
		if(load_image(cube + i, fname) == -1) {
			while(--i >= 0) {
				destroy_image(cube + i);
			}
			return -1;
		}
	}
	return 0;
}

void destroy_cubemap(struct image *cube)
{
	int i;

	if(!cube) return;

	for(i=0; i<6; i++) {
		destroy_image(cube + i);
	}
}
