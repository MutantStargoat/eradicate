#ifndef PLAYLIST_H_
#define PLAYLIST_H_

#include "audio.h"

struct playlist;

struct playlist *create_playlist(const char *plpath);
void destroy_playlist(struct playlist *pl);

void shuffle_playlist(struct playlist *pl);

int start_playlist(struct playlist *pl);
int cont_playlist(struct playlist *pl);
void stop_playlist(struct playlist *pl);
int next_playlist(struct playlist *pl);

struct au_module *playlist_current(struct playlist *pl);

/* call this often to keep playing music
 * returns time since last item started playing (ms), or -1
 */
long proc_playlist(struct playlist *pl);

#endif	/* PLAYLIST_H_ */
