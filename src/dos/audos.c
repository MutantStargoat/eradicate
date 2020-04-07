#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "midasdll.h"

static MIDASmodulePlayHandle modplay;
static struct au_module *curmod;
static int vol;

int au_init(void)
{
	modplay = 0;
	curmod = 0;
	vol = 64;

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

int au_load_module(struct au_module *mod, const char *fname)
{
	MIDASmoduleInfo info;
	const char *name;

	if(!(mod->impl = MIDASloadModule((char*)fname))) {
		fprintf(stderr, "au_load_module: failed to load module: %s\n", fname);
		return -1;
	}

	if(MIDASgetModuleInfo(mod->impl, &info) && info.songName[0]) {
		name = info.songName;
	} else {
		if((name = strrchr(fname, '/')) || (name = strrchr(fname, '\\'))) {
			name++;
		} else {
			name = fname;
		}
	}

	if(!(mod->name = malloc(strlen(name) + 1))) {
		fprintf(stderr, "au_load_module: mod->name malloc failed\n");
		MIDASfreeModule(mod->impl);
		return -1;
	}
	strcpy(mod->name, name);

	return 0;
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
	curmod = mod;
	return 0;
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

void au_music_volume(int v)
{
	v = v ? (v + 1) >> 2 : 0;

	if(vol == v) return;

	vol = v;
	if(curmod) {
		MIDASsetMusicVolume(modplay, v);
	}
}
