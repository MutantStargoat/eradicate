/* This file contains code to read and write four byte rgbe file format
 * developed by Greg Ward.  It handles the conversions between rgbe and
 * pixels consisting of floats.  The data is assumed to be an array of floats.
 * By default there are three floats per pixel in the order red, green, blue.
 * (RGBE_DATA_??? values control this.)
 *
 * written by Bruce Walter  (bjw@graphics.cornell.edu)  5/26/95
 * based on code written by Greg Ward
 * minor modifications by John Tsiombikas (nuclear@member.fsf.org) apr.9 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include "imago2.h"
#include "ftmodule.h"


typedef struct {
	int valid;				/* indicate which fields are valid */
	char programtype[16];	/* listed at beginning of file to identify it
							 * after "#?".  defaults to "RGBE" */
	float gamma;			/* image has already been gamma corrected with
							 * given gamma.  defaults to 1.0 (no correction) */
	float exposure;			/* a value of 1.0 in an image corresponds to
							 * <exposure> watts/steradian/m^2.
							 * defaults to 1.0 */
} rgbe_header_info;


static int check(struct img_io *io);
static int read(struct img_pixmap *img, struct img_io *io);
static int write(struct img_pixmap *img, struct img_io *io);

static int rgbe_read_header(struct img_io *io, int *width, int *height, rgbe_header_info * info);
static int rgbe_read_pixels_rle(struct img_io *io, float *data, int scanline_width, int num_scanlines);


int img_register_rgbe(void)
{
	static struct ftype_module mod = {".rgbe", check, read, write};
	return img_register_module(&mod);
}


static int check(struct img_io *io)
{
	int xsz, ysz, res;
	long pos = io->seek(0, SEEK_CUR, io->uptr);

	rgbe_header_info hdr;
	res = rgbe_read_header(io, &xsz, &ysz, &hdr);

	io->seek(pos, SEEK_SET, io->uptr);
	return res;
}

static int read(struct img_pixmap *img, struct img_io *io)
{
	int xsz, ysz;
	rgbe_header_info hdr;

	if(rgbe_read_header(io, &xsz, &ysz, &hdr) == -1) {
		return -1;
	}

	if(img_set_pixels(img, xsz, ysz, IMG_FMT_RGBF, 0) == -1) {
		return -1;
	}
	if(rgbe_read_pixels_rle(io, img->pixels, xsz, ysz) == -1) {
		return -1;
	}
	return 0;
}

static int write(struct img_pixmap *img, struct img_io *io)
{
	return -1;	/* TODO */
}


static int iofgetc(struct img_io *io)
{
	char c;
	return io->read(&c, 1, io->uptr) < 1 ? -1 : c;
}

static char *iofgets(char *buf, int size, struct img_io *io)
{
	int c;
	char *ptr = buf;

	while(--size > 0 && (c = iofgetc(io)) != -1) {
		*ptr++ = c;
		if(c == '\n') break;
	}
	*ptr = 0;

	return ptr == buf ? 0 : buf;
}


/* flags indicating which fields in an rgbe_header_info are valid */
#define RGBE_VALID_PROGRAMTYPE 0x01
#define RGBE_VALID_GAMMA       0x02
#define RGBE_VALID_EXPOSURE    0x04

/* return codes for rgbe routines */
#define RGBE_RETURN_SUCCESS 0
#define RGBE_RETURN_FAILURE -1


#if defined(__cplusplus) || defined(GNUC) || __STDC_VERSION >= 199901L
#define INLINE inline
#else
#define INLINE
#endif

/* offsets to red, green, and blue components in a data (float) pixel */
#define RGBE_DATA_RED	0
#define RGBE_DATA_GREEN  1
#define RGBE_DATA_BLUE   2

/* number of floats per pixel */
#define RGBE_DATA_SIZE   3

enum rgbe_error_codes {
	rgbe_read_error,
	rgbe_write_error,
	rgbe_format_error,
	rgbe_memory_error
};


/* default error routine.  change this to change error handling */
static int rgbe_error(int rgbe_error_code, char *msg)
{
	switch (rgbe_error_code) {
	case rgbe_read_error:
		fprintf(stderr, "RGBE read error: %s\n", strerror(errno));
		break;

	case rgbe_write_error:
		fprintf(stderr, "RGBE write error: %s\n", strerror(errno));
		break;

	case rgbe_format_error:
		fprintf(stderr, "RGBE bad file format: %s\n", msg);
		break;

	default:
	case rgbe_memory_error:
		fprintf(stderr, "RGBE error: %s\n", msg);
	}
	return RGBE_RETURN_FAILURE;
}

/* standard conversion from float pixels to rgbe pixels */
/*static INLINE void float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
	float v;
	int e;

	v = red;
	if(green > v)
		v = green;
	if(blue > v)
		v = blue;
	if(v < 1e-32) {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	} else {
		v = frexp(v, &e) * 256.0 / v;
		rgbe[0] = (unsigned char)(red * v);
		rgbe[1] = (unsigned char)(green * v);
		rgbe[2] = (unsigned char)(blue * v);
		rgbe[3] = (unsigned char)(e + 128);
	}
}*/

/* standard conversion from rgbe to float pixels */
/* note: Ward uses ldexp(col+0.5,exp-(128+8)). However we wanted pixels */
/*       in the range [0,1] to map back into the range [0,1]. */
static INLINE void rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4])
{
	float f;

	if(rgbe[3]) {				/*nonzero pixel */
		f = ldexp(1.0, rgbe[3] - (int)(128 + 8));
		*red = rgbe[0] * f;
		*green = rgbe[1] * f;
		*blue = rgbe[2] * f;
	} else
		*red = *green = *blue = 0.0;
}

#if 0
/* default minimal header. modify if you want more information in header */
static int rgbe_write_header(FILE * fp, int width, int height, rgbe_header_info * info)
{
	char *programtype = "RGBE";

	if(info && (info->valid & RGBE_VALID_PROGRAMTYPE))
		programtype = info->programtype;
	if(fprintf(fp, "#?%s\n", programtype) < 0)
		return rgbe_error(rgbe_write_error, NULL);
	/* The #? is to identify file type, the programtype is optional. */
	if(info && (info->valid & RGBE_VALID_GAMMA)) {
		if(fprintf(fp, "GAMMA=%g\n", info->gamma) < 0)
			return rgbe_error(rgbe_write_error, NULL);
	}
	if(info && (info->valid & RGBE_VALID_EXPOSURE)) {
		if(fprintf(fp, "EXPOSURE=%g\n", info->exposure) < 0)
			return rgbe_error(rgbe_write_error, NULL);
	}
	if(fprintf(fp, "FORMAT=32-bit_rle_rgbe\n\n") < 0)
		return rgbe_error(rgbe_write_error, NULL);
	if(fprintf(fp, "-Y %d +X %d\n", height, width) < 0)
		return rgbe_error(rgbe_write_error, NULL);
	return RGBE_RETURN_SUCCESS;
}
#endif

/* minimal header reading.  modify if you want to parse more information */
static int rgbe_read_header(struct img_io *io, int *width, int *height, rgbe_header_info * info)
{
	char buf[128];
	float tempf;
	int i;

	if(info) {
		info->valid = 0;
		info->programtype[0] = 0;
		info->gamma = info->exposure = 1.0;
	}
	if(iofgets(buf, sizeof(buf) / sizeof(buf[0]), io) == NULL)
		return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_read_error, NULL);*/
	if((buf[0] != '#') || (buf[1] != '?')) {
		/* if you want to require the magic token then uncomment the next line */
		/*return rgbe_error(rgbe_format_error,"bad initial token"); */
	} else if(info) {
		info->valid |= RGBE_VALID_PROGRAMTYPE;
		for(i = 0; i < sizeof(info->programtype) - 1; i++) {
			if((buf[i + 2] == 0) || isspace(buf[i + 2]))
				break;
			info->programtype[i] = buf[i + 2];
		}
		info->programtype[i] = 0;
		if(iofgets(buf, sizeof(buf) / sizeof(buf[0]), io) == 0)
			return rgbe_error(rgbe_read_error, NULL);
	}
	for(;;) {
		if((buf[0] == 0) || (buf[0] == '\n'))
			return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_format_error, "no FORMAT specifier found");*/
		else if(strcmp(buf, "FORMAT=32-bit_rle_rgbe\n") == 0)
			break;				/* format found so break out of loop */
		else if(info && (sscanf(buf, "GAMMA=%g", &tempf) == 1)) {
			info->gamma = tempf;
			info->valid |= RGBE_VALID_GAMMA;
		} else if(info && (sscanf(buf, "EXPOSURE=%g", &tempf) == 1)) {
			info->exposure = tempf;
			info->valid |= RGBE_VALID_EXPOSURE;
		}
		if(iofgets(buf, sizeof(buf) / sizeof(buf[0]), io) == 0)
			return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_read_error, NULL);*/
	}
	if(iofgets(buf, sizeof(buf) / sizeof(buf[0]), io) == 0)
		return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_read_error, NULL);*/
	if(strcmp(buf, "\n") != 0)
		return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_format_error, "missing blank line after FORMAT specifier");*/
	if(iofgets(buf, sizeof(buf) / sizeof(buf[0]), io) == 0)
		return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_read_error, NULL);*/
	if(sscanf(buf, "-Y %d +X %d", height, width) < 2)
		return RGBE_RETURN_FAILURE;/*rgbe_error(rgbe_format_error, "missing image size specifier");*/
	return RGBE_RETURN_SUCCESS;
}

#if 0
/* simple write routine that does not use run length encoding */

/* These routines can be made faster by allocating a larger buffer and
   fread-ing and fwrite-ing the data in larger chunks */
static int rgbe_write_pixels(FILE * fp, float *data, int numpixels)
{
	unsigned char rgbe[4];

	while(numpixels-- > 0) {
		float2rgbe(rgbe, data[RGBE_DATA_RED], data[RGBE_DATA_GREEN], data[RGBE_DATA_BLUE]);
		data += RGBE_DATA_SIZE;
		if(fwrite(rgbe, sizeof(rgbe), 1, fp) < 1)
			return rgbe_error(rgbe_write_error, NULL);
	}
	return RGBE_RETURN_SUCCESS;
}
#endif

/* simple read routine.  will not correctly handle run length encoding */
static int rgbe_read_pixels(struct img_io *io, float *data, int numpixels)
{
	unsigned char rgbe[4];

	while(numpixels-- > 0) {
		if(io->read(rgbe, sizeof(rgbe), io->uptr) < 1)
			return rgbe_error(rgbe_read_error, NULL);
		rgbe2float(&data[RGBE_DATA_RED], &data[RGBE_DATA_GREEN], &data[RGBE_DATA_BLUE], rgbe);
		data += RGBE_DATA_SIZE;
	}
	return RGBE_RETURN_SUCCESS;
}

#if 0
/* The code below is only needed for the run-length encoded files. */

/* Run length encoding adds considerable complexity but does */

/* save some space.  For each scanline, each channel (r,g,b,e) is */

/* encoded separately for better compression. */

static int rgbe_write_bytes_rle(struct img_io *io, unsigned char *data, int numbytes)
{
#define MINRUNLENGTH 4
	int cur, beg_run, run_count, old_run_count, nonrun_count;
	unsigned char buf[2];

	cur = 0;
	while(cur < numbytes) {
		beg_run = cur;
		/* find next run of length at least 4 if one exists */
		run_count = old_run_count = 0;
		while((run_count < MINRUNLENGTH) && (beg_run < numbytes)) {
			beg_run += run_count;
			old_run_count = run_count;
			run_count = 1;
			while((beg_run + run_count < numbytes) && (run_count < 127)
				  && (data[beg_run] == data[beg_run + run_count]))
				run_count++;
		}
		/* if data before next big run is a short run then write it as such */
		if((old_run_count > 1) && (old_run_count == beg_run - cur)) {
			buf[0] = 128 + old_run_count;	/*write short run */
			buf[1] = data[cur];
			if(fwrite(buf, sizeof(buf[0]) * 2, 1, fp) < 1)
				return rgbe_error(rgbe_write_error, NULL);
			cur = beg_run;
		}
		/* write out bytes until we reach the start of the next run */
		while(cur < beg_run) {
			nonrun_count = beg_run - cur;
			if(nonrun_count > 128)
				nonrun_count = 128;
			buf[0] = nonrun_count;
			if(fwrite(buf, sizeof(buf[0]), 1, fp) < 1)
				return rgbe_error(rgbe_write_error, NULL);
			if(fwrite(&data[cur], sizeof(data[0]) * nonrun_count, 1, fp) < 1)
				return rgbe_error(rgbe_write_error, NULL);
			cur += nonrun_count;
		}
		/* write out next run if one was found */
		if(run_count >= MINRUNLENGTH) {
			buf[0] = 128 + run_count;
			buf[1] = data[beg_run];
			if(fwrite(buf, sizeof(buf[0]) * 2, 1, fp) < 1)
				return rgbe_error(rgbe_write_error, NULL);
			cur += run_count;
		}
	}
	return RGBE_RETURN_SUCCESS;
#undef MINRUNLENGTH
}

static int rgbe_write_pixels_rle(struct img_io *io, float *data, int scanline_width, int num_scanlines)
{
	unsigned char rgbe[4];
	unsigned char *buffer;
	int i, err;

	if((scanline_width < 8) || (scanline_width > 0x7fff))
		/* run length encoding is not allowed so write flat */
		return rgbe_write_pixels(io, data, scanline_width * num_scanlines);
	buffer = (unsigned char *)malloc(sizeof(unsigned char) * 4 * scanline_width);
	if(buffer == NULL)
		/* no buffer space so write flat */
		return rgbe_write_pixels(fp, data, scanline_width * num_scanlines);
	while(num_scanlines-- > 0) {
		rgbe[0] = 2;
		rgbe[1] = 2;
		rgbe[2] = scanline_width >> 8;
		rgbe[3] = scanline_width & 0xFF;
		if(fwrite(rgbe, sizeof(rgbe), 1, fp) < 1) {
			free(buffer);
			return rgbe_error(rgbe_write_error, NULL);
		}
		for(i = 0; i < scanline_width; i++) {
			float2rgbe(rgbe, data[RGBE_DATA_RED], data[RGBE_DATA_GREEN], data[RGBE_DATA_BLUE]);
			buffer[i] = rgbe[0];
			buffer[i + scanline_width] = rgbe[1];
			buffer[i + 2 * scanline_width] = rgbe[2];
			buffer[i + 3 * scanline_width] = rgbe[3];
			data += RGBE_DATA_SIZE;
		}
		/* write out each of the four channels separately run length encoded */
		/* first red, then green, then blue, then exponent */
		for(i = 0; i < 4; i++) {
			if((err = rgbe_write_bytes_rle(fp, &buffer[i * scanline_width],
										  scanline_width)) != RGBE_RETURN_SUCCESS) {
				free(buffer);
				return err;
			}
		}
	}
	free(buffer);
	return RGBE_RETURN_SUCCESS;
}
#endif

static int rgbe_read_pixels_rle(struct img_io *io, float *data, int scanline_width, int num_scanlines)
{
	unsigned char rgbe[4], *scanline_buffer, *ptr, *ptr_end;
	int i, count;
	unsigned char buf[2];

	if((scanline_width < 8) || (scanline_width > 0x7fff))
		/* run length encoding is not allowed so read flat */
		return rgbe_read_pixels(io, data, scanline_width * num_scanlines);
	scanline_buffer = NULL;
	/* read in each successive scanline */
	while(num_scanlines > 0) {
		if(io->read(rgbe, sizeof(rgbe), io->uptr) < 1) {
			free(scanline_buffer);
			return rgbe_error(rgbe_read_error, NULL);
		}
		if((rgbe[0] != 2) || (rgbe[1] != 2) || (rgbe[2] & 0x80)) {
			/* this file is not run length encoded */
			rgbe2float(&data[0], &data[1], &data[2], rgbe);
			data += RGBE_DATA_SIZE;
			free(scanline_buffer);
			return rgbe_read_pixels(io, data, scanline_width * num_scanlines - 1);
		}
		if((((int)rgbe[2]) << 8 | rgbe[3]) != scanline_width) {
			free(scanline_buffer);
			return rgbe_error(rgbe_format_error, "wrong scanline width");
		}
		if(scanline_buffer == NULL)
			scanline_buffer = (unsigned char *)
				malloc(sizeof(unsigned char) * 4 * scanline_width);
		if(scanline_buffer == NULL)
			return rgbe_error(rgbe_memory_error, "unable to allocate buffer space");

		ptr = &scanline_buffer[0];
		/* read each of the four channels for the scanline into the buffer */
		for(i = 0; i < 4; i++) {
			ptr_end = &scanline_buffer[(i + 1) * scanline_width];
			while(ptr < ptr_end) {
				if(io->read(buf, sizeof(buf[0]) * 2, io->uptr) < 1) {
					free(scanline_buffer);
					return rgbe_error(rgbe_read_error, NULL);
				}
				if(buf[0] > 128) {
					/* a run of the same value */
					count = buf[0] - 128;
					if((count == 0) || (count > ptr_end - ptr)) {
						free(scanline_buffer);
						return rgbe_error(rgbe_format_error, "bad scanline data");
					}
					while(count-- > 0)
						*ptr++ = buf[1];
				} else {
					/* a non-run */
					count = buf[0];
					if((count == 0) || (count > ptr_end - ptr)) {
						free(scanline_buffer);
						return rgbe_error(rgbe_format_error, "bad scanline data");
					}
					*ptr++ = buf[1];
					if(--count > 0) {
						if(io->read(ptr, sizeof(*ptr) * count, io->uptr) < 1) {
							free(scanline_buffer);
							return rgbe_error(rgbe_read_error, NULL);
						}
						ptr += count;
					}
				}
			}
		}
		/* now convert data from buffer into floats */
		for(i = 0; i < scanline_width; i++) {
			rgbe[0] = scanline_buffer[i];
			rgbe[1] = scanline_buffer[i + scanline_width];
			rgbe[2] = scanline_buffer[i + 2 * scanline_width];
			rgbe[3] = scanline_buffer[i + 3 * scanline_width];
			rgbe2float(&data[RGBE_DATA_RED], &data[RGBE_DATA_GREEN], &data[RGBE_DATA_BLUE], rgbe);
			data += RGBE_DATA_SIZE;
		}
		num_scanlines--;
	}
	free(scanline_buffer);
	return RGBE_RETURN_SUCCESS;
}
