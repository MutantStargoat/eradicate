#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <alloca.h>
#include "mikmod.h"
#include "audio.h"

#define SET_MUS_VOL(vol) \
	do { \
		int mv = (vol) * vol_master >> 9; \
		Player_SetVolume(mv ? mv + 1 : 0); \
	} while(0)

static struct au_module *curmod;
static int vol_master, vol_mus, vol_sfx;

int au_init(void)
{
	curmod = 0;
	vol_master = vol_mus = vol_sfx = 255;

	//MikMod_RegisterDriver(&drv_sdl);
	MikMod_RegisterDriver(&drv_nos);

	MikMod_RegisterLoader(&load_it);
	MikMod_RegisterLoader(&load_mod);
	MikMod_RegisterLoader(&load_s3m);
	MikMod_RegisterLoader(&load_xm);

	if(MikMod_Init("")) {
		fprintf(stderr, "failed ot initialize mikmod: %s\n", MikMod_strerror(MikMod_errno));
		return -1;
	}
	return 0;
}

void au_shutdown(void)
{
	MikMod_Exit();
}

struct au_module *au_load_module(const char *fname)
{
	struct au_module *mod;
	MODULE *mikmod;
	char *name = 0, *end;

	if(!(mod = malloc(sizeof *mod))) {
		fprintf(stderr, "au_load_module: failed to allocate module\n");
		return 0;
	}

	if(!(mikmod = Player_Load(fname, 128, 0))) {
		fprintf(stderr, "au_load_module: failed to load module: %s: %s\n",
				fname, MikMod_strerror(MikMod_errno));
		free(mod);
		return 0;
	}
	mod->impl = mikmod;

	if(mikmod->songname && *mikmod->songname) {
		name = alloca(strlen(mikmod->songname) + 1);
		strcpy(name, mikmod->songname);

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
		Player_Free(mod->impl);
		free(mod);
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
	Player_Free(mod->impl);
	free(mod->name);
	free(mod);
}

int au_play_module(struct au_module *mod)
{
	if(curmod) {
		if(curmod == mod) return 0;
		au_stop_module(curmod);
	}

	Player_Start(mod->impl);
	SET_MUS_VOL(vol_mus);
	curmod = mod;
	return 0;
}

void au_update(void)
{
	if(!curmod) return;

	if(Player_Active()) {
		MikMod_Update();
	} else {
		Player_Stop();
		curmod = 0;
	}
}

int au_stop_module(struct au_module *mod)
{
	if(mod && curmod != mod) return -1;
	if(!curmod) return -1;

	Player_Stop();
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
	/* TODO */
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
