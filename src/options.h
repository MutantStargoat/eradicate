#ifndef OPTIONS_H_
#define OPTIONS_H_

#ifndef MSDOS
enum {
	SCALER_NEAREST,
	SCALER_LINEAR,

	NUM_SCALERS
};
#endif

struct joystick {
	int xmin, xmax;
	int ymin, ymax;
};

struct options {
	int xres, yres, bpp;
	int vsync;
	int viewdist;
	int vol_master, vol_mus, vol_sfx;
	int music;
	struct joystick jscal;
#ifndef MSDOS
	int fullscreen;
	int scaler;
#endif
};

extern struct options opt;

int load_options(const char *fname);
int save_options(const char *fname);

#endif	/* OPTIONS_H_ */
