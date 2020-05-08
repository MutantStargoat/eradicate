#ifndef RESMAN_H_
#define RESMAN_H_

#include "image.h"

void *create_imgset(void);
void free_imgset(void *set);
int imgset_add(void *set, struct image *img, const char *name);
struct image *imgset_get(void *set, const char *name);

#endif	/* RESMAN_H_ */
