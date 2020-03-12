#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "options.h"
#include "treestor.h"

#define DEF_XRES	320
#define DEF_YRES	240
#define DEF_BPP		16

struct options opt = {
	DEF_XRES, DEF_YRES, DEF_BPP
};

int load_options(const char *fname)
{
	struct ts_node *cfg;

	if(!(cfg = ts_load(fname))) {
		return -1;
	}

	opt.xres = ts_lookup_int(cfg, "gfx.xres", DEF_XRES);
	opt.yres = ts_lookup_int(cfg, "gfx.yres", DEF_YRES);
	opt.bpp = ts_lookup_int(cfg, "gfx.bpp", DEF_BPP);

	ts_free_tree(cfg);
	return 0;
}

int save_options(const char *fname)
{
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "failed to save options (%s): %s\n", fname, strerror(errno));
	}
	fprintf(fp, "gfx {\n");
	if(opt.xres != DEF_XRES || opt.yres != DEF_YRES) {
		fprintf(fp, "\txres = %d\n", opt.xres);
		fprintf(fp, "\tyres = %d\n", opt.yres);
	}
	if(opt.bpp != DEF_BPP) {
		fprintf(fp, "\tbpp = %d\n", opt.bpp);
	}
	fprintf(fp, "}\n");

	fclose(fp);
	return 0;
}
