#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <imago2.h>
#include "image.h"
#include "treestor.h"

int load_image(struct image *img, const char *fname)
{
	FILE *fp;
	char sig[8];
	uint16_t width, height;

	memset(img, 0, sizeof *img);

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
		goto not565;
	}

	/* it's a 565 dump, read it and return */
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
not565:
	fclose(fp);

	if(memcmp(sig, "animtex", 7) == 0) {
		/* it's an animated texture. read metadata, and recurse with the new name*/
		struct ts_node *root, *node;
		const char *imgfname;
		int fps, num_frames = 0;

		if(!(root = ts_load(fname))) {
			fprintf(stderr, "failed to load animation %s\n", fname);
			return -1;
		}
		if(!(imgfname = ts_lookup_str(root, "animtex.image", 0))) {
			fprintf(stderr, "animtex %s missing `image` attribute\n", fname);
			ts_free_tree(root);
			return -1;
		}
		if(strcmp(imgfname, fname) == 0) {
			fprintf(stderr, "animtex %s is silly...\n", fname);
			ts_free_tree(root);
			return -1;
		}

		if(load_image(img, imgfname) == -1) {
			ts_free_tree(root);
			return -1;
		}

		fps = ts_lookup_int(root, "animtex.framerate", 25);
		img->frame_interval = 1.0f / (float)fps;

		/* count frames */
		node = root->child_list;
		while(node) {
			if(strcmp(node->name, "frame") == 0) {
				num_frames++;
			}
			node = node->next;
		}

		if(!(img->uoffs = malloc(2 * num_frames * sizeof *img->uoffs))) {
			fprintf(stderr, "animtex %s: failed to allocate uvoffset array for %d frames\n", fname, num_frames);
			free(img->pixels);
			ts_free_tree(root);
			return -1;
		}
		img->voffs = img->uoffs + num_frames;

		num_frames = 0;
		node = root->child_list;
		while(node) {
			if(strcmp(node->name, "frame") == 0) {
				float *v = ts_get_attr_vec(node, "uvoffset", 0);
				if(v) {
					img->uoffs[num_frames] = v[0];
					img->voffs[num_frames++] = v[1];
				} else {
					fprintf(stderr, "animtex %s: ignoring frame without uvoffset\n", fname);
				}
			}
			node = node->next;
		}

		img->num_frames = num_frames;

		ts_free_tree(root);
		return 0;
	}

	/* just a regular image */
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
