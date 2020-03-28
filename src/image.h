#ifndef IMAGE_H_
#define IMAGE_H_

#include "inttypes.h"

struct image {
	int width, height;
	uint16_t *pixels;
};

int load_image(struct image *img, const char *fname);
int dump_image(struct image *img, const char *fname);
void destroy_image(struct image *img);

int load_cubemap(struct image *cube, const char *fname_fmt);
void destroy_cubemap(struct image *cube);

#endif	/* IMAGE_H_ */
