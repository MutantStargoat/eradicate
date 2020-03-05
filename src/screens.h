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

#endif	/* SCREENS_H_ */