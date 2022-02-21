#ifndef GFX_H_
#define GFX_H_

#include "inttypes.h"

struct video_mode {
	uint16_t mode;
	short xsz, ysz, bpp, pitch;
	short rbits, gbits, bbits;
	short rshift, gshift, bshift;
	uint32_t rmask, gmask, bmask;
	uint32_t fb_addr;
	short max_pages;
	uint32_t bank_size;
	void *data;
};

int init_video(void);
void cleanup_video(void);

struct video_mode *video_modes(void);
int num_video_modes(void);

#define VMODE_CURRENT	(-1)
struct video_mode *get_video_mode(int idx);

int match_video_mode(int xsz, int ysz, int bpp);

/* argument is the mode list index [0, nmodes-1] */
void *set_video_mode(int idx, int nbuf);
int set_text_mode(void);

void blit_frame(void *pixels, int vsync);
void wait_vsync(void);

#endif	/* GFX_H_ */
