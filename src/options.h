#ifndef OPTIONS_H_
#define OPTIONS_H_

struct options {
	int xres, yres, bpp;
	int vol_master, vol_mus, vol_sfx;
};

extern struct options opt;

int load_options(const char *fname);
int save_options(const char *fname);

#endif	/* OPTIONS_H_ */
