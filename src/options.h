#ifndef OPTIONS_H_
#define OPTIONS_H_

struct options {
	int xres, yres, bpp;
};

extern struct options opt;

int load_options(const char *fname);
int save_options(const char *fname);

#endif	/* OPTIONS_H_ */
