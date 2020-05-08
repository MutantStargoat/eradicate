#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rbtree.h"
#include "image.h"

static void imgdel(struct rbnode *node, void *cls);

struct rbtree *create_imgset(void)
{
	struct rbtree *rb;

	if(!(rb = rb_create(RB_KEY_STRING))) {
		fprintf(stderr, "create_imgset: failed to allocate set\n");
		return 0;
	}
	rb_set_delete_func(rb, imgdel, 0);

	return rb;
}

void free_imgset(struct rbtree *rb)
{
	rb_free(rb);
}

int imgset_add(struct rbtree *rb, struct image *img, const char *name)
{
	return rb_insert(rb, strdup(name), img);
}

struct image *imgset_get(struct rbtree *rb, const char *name)
{
	struct rbnode *node;
	struct image *img;

	if((node = rb_find(rb, (char*)name))) {
		return node->data;
	}

	if(!(img = malloc(sizeof *img))) {
		fprintf(stderr, "failed to allocate image\n");
		return 0;
	}
	if(load_image(img, name) == -1) {
		fprintf(stderr, "failed to load image: %s\n", name);
		free(img);
		return 0;
	}
	printf("imgset: loaded image %s (%dx%d)\n", name, img->width, img->height);
	imgset_add(rb, img, name);
	return img;
}

static void imgdel(struct rbnode *node, void *cls)
{
	free(node->key);
	free(node->data);
}
