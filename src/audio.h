#ifndef AUDIO_H_
#define AUDIO_H_

enum { AU_STOPPED, AU_PLAYING };

struct au_module {
	char *name;
	void *impl;
};

int au_init(void);
void au_shutdown(void);

struct au_module *au_load_module(const char *fname);
void au_free_module(struct au_module *mod);

int au_play_module(struct au_module *mod);
void au_update(void);
int au_stop_module(struct au_module *mod);
int au_module_state(struct au_module *mod);

void au_music_volume(int vol);

#endif	/* AUDIO_H_ */
