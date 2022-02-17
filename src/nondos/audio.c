#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if defined(__WATCOMC__) || defined(_WIN32) || defined(__DJGPP__)
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include "mikmod.h"
#include "audio.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#define SET_MUS_VOL(vol) \
	do { \
		int mv = (vol) * vol_master >> 9; \
		Player_SetVolume((SWORD)(mv ? mv + 1 : 0)); \
	} while(0)

static struct au_module *curmod;
static int vol_master, vol_mus, vol_sfx;

#ifdef _WIN32
static DWORD WINAPI update(void *cls);
#else
static void *update(void *cls);
#endif

int au_init(void)
{
	curmod = 0;
	vol_master = vol_mus = vol_sfx = 255;

#if defined(__linux__)
	MikMod_RegisterDriver(&drv_alsa);
#elif defined(__FreeBSD__)
	MikMod_RegisterDriver(&drv_oss);
#elif defined(__sgi__)
	MikMod_RegisterDriver(&drv_sgi);
#elif defined(_WIN32)
	MikMod_RegisterDriver(&drv_ds);
#else
	MikMod_RegisterDriver(&drv_nos);
#endif

	MikMod_RegisterLoader(&load_it);
	MikMod_RegisterLoader(&load_mod);
	MikMod_RegisterLoader(&load_s3m);
	MikMod_RegisterLoader(&load_xm);

	if(MikMod_Init("")) {
		fprintf(stderr, "failed ot initialize mikmod: %s\n", MikMod_strerror(MikMod_errno));
		return -1;
	}
	MikMod_InitThreads();

	{
#ifdef _WIN32
		HANDLE thr;
		if((thr = CreateThread(0, 0, update, 0, 0, 0))) {
			CloseHandle(thr);
		}
#else
		pthread_t upd_thread;
		if(pthread_create(&upd_thread, 0, update, 0) == 0) {
			pthread_detach(upd_thread);
		}
#endif
	}
	return 0;
}

void au_shutdown(void)
{
	curmod = 0;
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

	if(!Player_Active()) {
		Player_Stop();
		curmod = 0;
	}
}

#ifdef _WIN32
static DWORD WINAPI update(void *cls)
#else
static void *update(void *cls)
#endif
{
	for(;;) {
		if(Player_Active()) {
			MikMod_Update();
		}
#ifdef _WIN32
		Sleep(10);
#else
		usleep(10000);
#endif
	}
	return 0;
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
