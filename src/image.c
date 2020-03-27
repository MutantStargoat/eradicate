#include <imago2.h>
#include "image.h"

int load_image(struct image *img, const char *fname)
{
	if(!(img->pixels = img_load_pixels(fname, &img->width, &img->height, IMG_FMT_RGB565))) {
		fprintf(stderr, "failed to load image: %s\n", fname);
		return -1;
	}
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
