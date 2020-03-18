#ifndef IMAGE_H_
#define IMAGE_H_

struct cmapent {
	unsigned char r, g, b;
};

struct image {
	int width, height;
	int bpp;
	int nchan;
	int scansz, pitch;
	int cmap_ncolors;
	struct cmapent cmap[256];
	unsigned char *pixels;
};

int alloc_image(struct image *img, int x, int y, int bpp);
int load_image(struct image *img, const char *fname);
int save_image(struct image *img, const char *fname);

int conv_image_rgb565(struct image *img16, struct image *img);

int cmp_image(struct image *a, struct image *b);

void blit_image(struct image *src, int sx, int sy, int w, int h, struct image *dst, int dx, int dy);

void image_color_offset(struct image *img, int offs);

#endif	/* IMAGE_H_ */
