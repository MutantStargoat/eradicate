#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199900
#include <stdint.h>
#else
#include <sys/types.h>
#endif
#include <assert.h>
#include <alloca.h>
#include "image.h"

#define MAGIC	"RLESPRIT"

#define BSWAP16(x)	((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

struct file_header {
	char magic[8];
	uint16_t width, height, bpp;
	uint16_t count;
} __attribute__((packed));

enum {
	CSOP_END,
	CSOP_ENDL,
	CSOP_SKIP,
	CSOP_FILL,
	CSOP_COPY
};

struct csop {
	unsigned char op;
	unsigned char padding;
	uint16_t len;
} __attribute__((packed));


struct rect {
	int x, y, w, h;
};

int rlesprite(struct image *img, int x, int y, int xsz, int ysz);
int proc_sheet(const char *fname);
void print_usage(const char *argv0);

int tile_xsz, tile_ysz;
struct rect rect;
int cmap_offs;
int ckey;
int conv565;
int padding;
FILE *outfp;

int main(int argc, char **argv)
{
	int i;
	char *endp;
	const char *name = 0;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-size") == 0) {
				if(sscanf(argv[++i], "%dx%d", &tile_xsz, &tile_ysz) != 2 ||
						tile_xsz <= 0 || tile_ysz <= 0) {
					fprintf(stderr, "%s must be followed by WIDTHxHEIGHT\n", argv[i - 1]);
					return 1;
				}

			} else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-rect") == 0) {
				rect.x = rect.y = 0;
				if(sscanf(argv[++i], "%dx%d+%d+%d", &rect.w, &rect.h, &rect.x, &rect.y) < 2 || rect.w <= 0 || rect.h <= 0) {
					fprintf(stderr, "%s must be followed by WIDTHxHEIGHT+X+Y\n", argv[i - 1]);
					return 1;
				}

			} else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-pad") == 0) {
				padding = strtol(argv[++i], &endp, 10);
				if(endp == argv[i] || padding < 0) {
					fprintf(stderr, "%s must be followed by a positive number\n", argv[i - 1]);
					return 1;
				}

			} else if(strcmp(argv[i], "-coffset") == 0) {
				cmap_offs = strtol(argv[++i], &endp, 10);
				if(endp == argv[i] || cmap_offs < 0 || cmap_offs >= 256) {
					fprintf(stderr, "-coffset must be followed by a valid colormap offset\n");
					return 1;
				}

			} else if(strcmp(argv[i], "-o") == 0) {
				if(!argv[++i]) {
					fprintf(stderr, "%s must be followed by an output filename\n", argv[i - 1]);
					return 1;
				}
				if(name && strcmp(name, argv[i]) != 0) {
					if(outfp) {
						fclose(outfp);
						outfp = 0;
					}
				}
				name = argv[i];

			} else if(strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "-key") == 0) {
				ckey = strtol(argv[++i], &endp, 10);
				if(endp == argv[i] || ckey < 0) {
					fprintf(stderr, "%s must be followed by a valid color key\n", argv[i - 1]);
					return 1;
				}

			} else if(strcmp(argv[i], "-conv565") == 0) {
				conv565 = 1;

			} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
				print_usage(argv[0]);
				return 0;

			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				print_usage(argv[0]);
				return 1;
			}

		} else {
			if(!outfp) {
				if(name) {
					if(!(outfp = fopen(name, "wb"))) {
						fprintf(stderr, "failed to open output file: %s: %s\n", name, strerror(errno));
						return 1;
					}
				} else {
					outfp = stdout;
				}
			}

			if(proc_sheet(argv[i]) == -1) {
				return 1;
			}
		}
	}

	return 0;
}

int proc_sheet(const char *fname)
{
	int i, j, x, y, num_xtiles, num_ytiles, xsz, ysz;
	struct image img;
	struct file_header hdr;

	if(load_image(&img, fname) == -1) {
		fprintf(stderr, "failed to load image: %s\n", fname);
		return -1;
	}
	if(conv565) {
		struct image tmp;
		if(conv_image_rgb565(&tmp, &img) == -1) {
			fprintf(stderr, "failed to convert image to 16bpp 565: %s\n", fname);
			free(img.pixels);
			return -1;
		}
		free(img.pixels);
		img = tmp;
	}

	if(rect.w <= 0) {
		rect.w = img.width;
		rect.h = img.height;
	}

	if(tile_xsz <= 0) {
		num_xtiles = num_ytiles = 1;
		xsz = rect.w - padding;
		ysz = rect.h - padding;
	} else {
		if(padding) {
			num_xtiles = num_ytiles = 0;
			i = 0;
			while(i < rect.w) {
				num_xtiles++;
				i += tile_xsz + padding;
			}
			i = 0;
			while(i < rect.h) {
				num_ytiles++;
				i += tile_ysz + padding;
			}
		} else {
			num_xtiles = rect.w / tile_xsz;
			num_ytiles = rect.h / tile_ysz;
		}
		xsz = tile_xsz;
		ysz = tile_ysz;
	}

	memcpy(hdr.magic, MAGIC, sizeof hdr.magic);
#ifdef BUILD_BIGENDIAN
	hdr.width = BSWAP16(xsz);
	hdr.height = BSWAP16(ysz);
	hdr.bpp = BSWAP16(img.bpp);
	hdr.count = BSWAP16(num_xtiles * num_ytiles);
#else
	hdr.width = xsz;
	hdr.height = ysz;
	hdr.bpp = img.bpp;
	hdr.count = num_xtiles * num_ytiles;
#endif
	fwrite(&hdr, sizeof hdr, 1, outfp);

	y = rect.y;
	for(i=0; i<num_ytiles; i++) {
		x = rect.x;
		for(j=0; j<num_xtiles; j++) {
			rlesprite(&img, x, y, xsz, ysz);
			x += xsz + padding;
		}
		y += ysz + padding;
	}

	free(img.pixels);
	return 0;
}

static void write_csop(struct csop *op)
{
#ifdef BUILD_BIGENDIAN
	struct csop tmp;
	tmp.len = BSWAP16(op->len);
	op = &tmp;
#endif
	fwrite(op, sizeof *op, 1, outfp);
}

int rlesprite(struct image *img, int x, int y, int xsz, int ysz)
{
	int i, j, numops, mode, new_mode, start, skip_acc, pixsz = img->bpp / 8;
	unsigned char *scanptr, *pptr;
	struct csop *ops, *optr, endop = {0};

	pptr = img->pixels + y * img->scansz + x * pixsz;

	ops = optr = alloca((xsz + 1) * ysz * sizeof *ops);

	for(i=0; i<ysz; i++) {
		mode = -1;
		start = -1;

		if(i > 0) {
			optr++->op = CSOP_ENDL;
		}

		for(j=0; j<xsz; j++) {
			if(memcmp(pptr, &ckey, pixsz) == 0) {
				new_mode = CSOP_SKIP;
			} else {
				new_mode = CSOP_COPY;
			}

			if(new_mode != mode) {
				if(mode != -1) {
					assert(start >= 0);
					optr->op = mode;
					optr->len = j - start;
					optr++;
				}
				mode = new_mode;
				start = j;
			}
			pptr += pixsz;
		}
		pptr += img->scansz - xsz * pixsz;

		if(mode != -1) {
			assert(start >= 0);
			optr->op = mode;
			optr->len = xsz - start;
			optr++;
		}
	}
	numops = optr - ops;

	scanptr = pptr = img->pixels + y * img->scansz + x * img->bpp / 8;
	optr = ops;
	skip_acc = 0;

	for(i=0; i<numops; i++) {
		switch(optr->op) {
		case CSOP_SKIP:
			skip_acc += optr->len;
			pptr += optr->len * pixsz;
			break;

		case CSOP_ENDL:
			/* maybe at some point combine multiple endl into yskips? meh */
			write_csop(optr);
			skip_acc = 0;
			scanptr += img->scansz;
			pptr = scanptr;
			break;

		case CSOP_COPY:
			if(skip_acc) {
				struct csop skip = {0};
				skip.op = CSOP_SKIP;
				skip.len = skip_acc;
				write_csop(&skip);
				skip_acc = 0;
			}

			write_csop(optr);
			fwrite(pptr, pixsz, optr->len, outfp);

			pptr += optr->len * pixsz;
			break;

		default:
			fprintf(stderr, "invalid op\n");
		}
		optr++;
	}

	endop.op = CSOP_END;
	write_csop(&endop);
	return 0;
}

void print_usage(const char *argv0)
{
	printf("Usage: %s [options] <spritesheet>\n", argv0);
	printf("Options:\n");
	printf(" -o <filename>: output filename (default: stdout)\n");
	printf(" -s,-size <WxH>: tile size (default: whole image)\n");
	printf(" -r,-rect <WxH+X+Y>: use rectangle of the input image (default: whole image)\n");
	printf(" -p,-pad <N>: how many pixels to skip between tiles in source image (default: 0)\n");
	printf(" -coffset <offs>: colormap offset [0, 255] (default: 0)\n");
	printf(" -k,-key <color>: color-key for transparency (default: 0)\n");
	printf(" -conv565: convert image to 16bpp 565 before processing\n");
	printf(" -h: print usage and exit\n");
}
