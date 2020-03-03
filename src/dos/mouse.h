#ifndef MOUSE_H_
#define MOUSE_H_

enum {
	MOUSE_LEFT		= 1,
	MOUSE_RIGHT		= 2,
	MOUSE_MIDDLE	= 4
};

#ifdef __cplusplus
extern "C" {
#endif

int have_mouse(void);
void show_mouse(int show);
int read_mouse(int *xp, int *yp);
void set_mouse(int x, int y);
void set_mouse_limits(int xmin, int ymin, int xmax, int ymax);
void set_mouse_rate(int xrate, int yrate);

#ifdef __cplusplus
}
#endif

#endif	/* MOUSE_H_ */
