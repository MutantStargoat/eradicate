#ifndef SCREENS_H_
#define SCREENS_H_

int intro_init(void);
void intro_cleanup(void);
void intro_start(void);
void intro_stop(void);
void intro_draw(void);
void intro_keyb(int key, int pressed);

int menu_init(void);
void menu_cleanup(void);
void menu_start(void);
void menu_stop(void);
void menu_draw(void);
void menu_keyb(int key, int pressed);

int race_init(void);
void race_cleanup(void);
void race_start(void);
void race_stop(void);
void race_draw(void);
void race_keyb(int key, int pressed);

int options_init(void);
void options_cleanup(void);
void options_start(void);
void options_stop(void);
void options_draw(void);
void options_keyb(int key, int pressed);

#endif	/* SCREENS_H_ */
