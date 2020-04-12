#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "audio.h"
#include "midasdll.h"

#define SET_MUS_VOL(vol) \
	do { \
		int mv = (vol) * vol_master >> 10; \
		MIDASsetMusicVolume(modplay, mv ? mv + 1 : 0); \
	} while(0)

static MIDASmodulePlayHandle modplay;
static struct au_module *curmod;

static int vol_master, vol_mus, vol_sfx;

int au_init(void)
{
	modplay = 0;
	curmod = 0;
	vol_master = vol_mus = vol_sfx = 255;

	/* TODO: load volume levels from config
	 * load audio card config, autodetect, or present config dialog
	 */

	MIDASstartup();
	MIDASinit();

	MIDASstartBackgroundPlay(0);
	return 0;
}

void au_shutdown(void)
{
	if(curmod) {
		au_stop_module(curmod);
	}
	MIDASstopBackgroundPlay();
	MIDASclose();
}

struct au_module *au_load_module(const char *fname)
{
	static MIDASmoduleInfo info;
	struct au_module *mod;
	char *name, *end;

	if(!(mod = malloc(sizeof *mod))) {
		fprintf(stderr, "au_load_module: failed to allocate module\n");
		return 0;
	}

	if(!(mod->impl = MIDASloadModule((char*)fname))) {
		fprintf(stderr, "au_load_module: failed to load module: %s\n", fname);
		free(mod);
		return 0;
	}

	name = 0;
	if(MIDASgetModuleInfo(mod->impl, &info)) {
		name = info.songName;
		end = name + strlen(name) - 1;
		while(end >= name && isspace(*end)) {
			*end-- = 0;
		}
		if(!*name) name = 0;
	}

	if(!name) {
		/* fallback to using the filename */
		if((name = strrchr(fname, '/')) || (name = strrchr(fname, '\\'))) {
			name++;
		} else {
			name = (char*)fname;
		}
	}

	if(!(mod->name = malloc(strlen(name) + 1))) {
		fprintf(stderr, "au_load_module: mod->name malloc failed\n");
		MIDASfreeModule(mod->impl);
		return 0;
	}
	strcpy(mod->name, name);

	printf("loaded module \"%s\" (%s)\n", name, fname);
	return mod;
}

void au_free_module(struct au_module *mod)
{
	if(!mod) return;

	if(mod == curmod) {
		au_stop_module(curmod);
	}
	MIDASfreeModule(mod->impl);
	free(mod->name);
	free(mod);
}

int au_play_module(struct au_module *mod)
{
	if(curmod) {
		if(curmod == mod) return 0;
		au_stop_module(curmod);
	}

	if(!(modplay = MIDASplayModule(mod->impl, 1))) {
		fprintf(stderr, "au_play_module: failed to play module: %s\n", mod->name);
		return -1;
	}
	SET_MUS_VOL(vol_mus);
	curmod = mod;
	return 0;
}

void au_update(void)
{
}

int au_stop_module(struct au_module *mod)
{
	if(mod && curmod != mod) return -1;
	if(!curmod) return -1;

	MIDASstopModule(modplay);
	curmod = 0;
	return 0;
}

int au_module_state(struct au_module *mod)
{
	if(mod) {
		return curmod == mod ? AU_PLAYING : AU_STOPPED;
	}
	return curmod ? AU_PLAYING : AU_STOPPED;
}

int au_volume(int vol)
{
	AU_VOLADJ(vol_master, vol);
	if(vol != vol_master) {
		vol_master = vol;

		au_sfx_volume(vol_sfx);
		au_music_volume(vol_mus);
	}
	return vol_master;
}

int au_sfx_volume(int vol)
{
	AU_VOLADJ(vol_sfx, vol);
	vol_sfx = vol;

	/* TODO set sfx volume */
	return vol_sfx;
}


int au_music_volume(int vol)
{
	AU_VOLADJ(vol_mus, vol);
	vol_mus = vol;

	if(curmod) {
		SET_MUS_VOL(vol);
	}
	return vol_mus;
}

/* when using MIDAS, we can't access the PIT directly, so we don't build timer.c
 * and implement all the timer functions here, using MIDAS callbacks
 */
static volatile unsigned long ticks;
static unsigned long tick_interval;

static void MIDAS_CALL midas_timer(void)
{
	ticks++;
}

/* macro to divide and round to the nearest integer */
#define DIV_ROUND(a, b) \
	((a) / (b) + ((a) % (b)) / ((b) / 2))

void init_timer(int res_hz)
{
	MIDASsetTimerCallbacks(res_hz * 1000, 0, midas_timer, 0, 0);
	tick_interval = DIV_ROUND(1000, res_hz);
}

void reset_timer(void)
{
	ticks = 0;
}

unsigned long get_msec(void)
{
	return ticks * tick_interval;
}
