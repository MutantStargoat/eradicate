#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifdef __WATCOMC__
#include <direct.h>
#else
#include <dirent.h>
#endif
#include <sys/stat.h>
#include "playlist.h"
#include "audio.h"
#include "game.h"
#include "fonts.h"

struct playlist {
	struct au_module *mod;
	char **files;
	int num_files;
	int cur;
	long cur_start_time;
};

static int ismodule(const char *fname);

struct playlist *create_playlist(const char *plpath)
{
	DIR *dir;
	struct dirent *dent;
	struct stat st;
	struct playlist *pl;
	char *fname;
	int plpath_len, max_files = 0;

	plpath_len = strlen(plpath);

	if(!(dir = opendir(plpath))) {
		fprintf(stderr, "create_playlist: failed to open directory: %s: %s\n", plpath, strerror(errno));
		return 0;
	}

	if(!(pl = calloc(1, sizeof *pl))) {
		fprintf(stderr, "create_playlist: failed to allocate playlist\n");
		closedir(dir);
		return 0;
	}
	pl->cur_start_time = -1;

	while((dent = readdir(dir))) {
		if(!ismodule(dent->d_name)) {
			continue;
		}

		if(!(fname = malloc(plpath_len + strlen(dent->d_name)))) {
			fprintf(stderr, "create_playlist: failed to allocate file name\n");
			break;
		}
		sprintf(fname, "%s/%s", plpath, dent->d_name);
		if(stat(fname, &st) != 0 || !S_ISREG(st.st_mode)) {
			continue;
		}

		/* it's (probably) a module */
		if(pl->num_files >= max_files) {
			char **tmp;
			int newmax = max_files ? max_files * 2 : 8;
			if(!(tmp = realloc(pl->files, newmax * sizeof *tmp))) {
				fprintf(stderr, "create_playlist: failed to resize module list\n");
				break;
			}
			pl->files = tmp;
			max_files = newmax;
		}

		pl->files[pl->num_files++] = fname;
	}

	closedir(dir);
	if(pl->num_files <= 0) {
		fprintf(stderr, "create_playlist: no modules found in %s\n", plpath);
		free(pl);
		return 0;
	}

	return pl;
}

void destroy_playlist(struct playlist *pl)
{
	int i;
	if(pl->mod) {
		au_free_module(pl->mod);
	}
	for(i=0; i<pl->num_files; i++) {
		free(pl->files[i]);
	}
	free(pl->files);
}

void shuffle_playlist(struct playlist *pl)
{
	int i;

	for(i=0; i<pl->num_files-1; i++) {
		int r = rand() % (pl->num_files - i);
		if(r != i) {
			char *tmp = pl->files[i];
			pl->files[i] = pl->files[r];
			pl->files[r] = tmp;
		}
	}
}

int start_playlist(struct playlist *pl)
{
	pl->cur = -1;
	return next_playlist(pl);
}

int cont_playlist(struct playlist *pl)
{
	return next_playlist(pl);
}

void stop_playlist(struct playlist *pl)
{
	if(pl->mod) {
		au_free_module(pl->mod);
		pl->mod = 0;
	}
}

int next_playlist(struct playlist *pl)
{
	int i;

	if(pl->mod) {
		if(pl->num_files <= 1 && au_module_state(pl->mod) == AU_PLAYING) {
			return 0;
		}
		au_free_module(pl->mod);
		pl->mod = 0;
	}

	for(i=0; i<pl->num_files; i++) {
		pl->cur = (pl->cur + 1) % pl->num_files;
		if((pl->mod = au_load_module(pl->files[pl->cur]))) {
			au_play_module(pl->mod);
			pl->cur_start_time = time_msec;
			return 0;
		}
	}
	return -1;
}

struct au_module *playlist_current(struct playlist *pl)
{
	return pl->mod;
}

#define TITLE_DUR		8192
#define TITLE_MOVE_DUR	(TITLE_DUR >> 2)
#define TITLE_OUT_START	(TITLE_DUR - TITLE_MOVE_DUR)
#define TITLE_HEIGHT	16

long proc_playlist(struct playlist *pl)
{
	long mus_time;

	draw_volume_bar(fb_pixels, fb_width - 80, 8);

	if(!pl->mod) return -1;

	au_update();

	if(au_module_state(pl->mod) == AU_STOPPED) {
		if(next_playlist(pl) == -1) {
			return -1;
		}
	}

	mus_time = time_msec - pl->cur_start_time;
	if(mus_time < TITLE_DUR) {
		int y;
		if(mus_time < TITLE_MOVE_DUR) {
			y = fb_height - TITLE_HEIGHT * mus_time / TITLE_MOVE_DUR;
		} else if(mus_time >= TITLE_OUT_START) {
			y = fb_height - TITLE_HEIGHT +
				TITLE_HEIGHT * (mus_time - TITLE_OUT_START) / TITLE_MOVE_DUR;
		} else {
			y = fb_height - TITLE_HEIGHT;
		}
		select_font(FONT_MENU);
		fnt_align(FONT_LEFT);
		fnt_print(fb_pixels, 5, y - 16, "now playing:");
		fnt_print(fb_pixels, 20, y, pl->mod->name);
	}
	return mus_time;
}

static int match_suffix(const char *str, const char *suffix)
{
	int len = strlen(str);
	int suflen = strlen(suffix);

	if(len < suflen) return 0;

	str += len - suflen;
	while(*str) {
		if(tolower(*str++) != tolower(*suffix++)) {
			return 0;
		}
	}
	return 1;
}

static int ismodule(const char *fname)
{
	static const char *suffix[] = {".mod", ".it", ".s3m", ".xm", 0};
	int i;

	for(i=0; suffix[i]; i++) {
		if(match_suffix(fname, suffix[i])) {
			return 1;
		}
	}
	return 0;
}
