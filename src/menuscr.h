#ifndef MENUSCR_H_
#define MENUSCR_H_

int menu_init(void);
void menu_cleanup(void);

void menu_start(void);
void menu_stop(void);

void menu_draw(void);
void menu_keyb(int key, int pressed);

#endif	/* MENUSCR_H_ */
