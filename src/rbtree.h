/*
rbtree - simple balanced binary search tree (red-black tree) library.
Copyright (C) 2011-2014  John Tsiombikas <nuclear@member.fsf.org>

rbtree is free software, feel free to use, modify, and redistribute it, under
the terms of the 3-clause BSD license. See COPYING for details.
 */
#ifndef RBTREE_H_
#define RBTREE_H_

struct rbtree;


struct rbnode {
	void *key, *data;
	int red;
	struct rbnode *left, *right;
	struct rbnode *next;	/* for iterator stack */
};


typedef void *(*rb_alloc_func_t)(size_t);
typedef void (*rb_free_func_t)(void*);

typedef int (*rb_cmp_func_t)(const void*, const void*);
typedef void (*rb_del_func_t)(struct rbnode*, void*);

#define RB_KEY_ADDR		(rb_cmp_func_t)(0)
#define RB_KEY_INT		(rb_cmp_func_t)(1)
#define RB_KEY_STRING	(rb_cmp_func_t)(3)


#ifdef __cplusplus
extern "C" {
#endif

struct rbtree *rb_create(rb_cmp_func_t cmp_func);
void rb_free(struct rbtree *rb);

int rb_init(struct rbtree *rb, rb_cmp_func_t cmp_func);
void rb_destroy(struct rbtree *rb);

void rb_set_allocator(struct rbtree *rb, rb_alloc_func_t alloc, rb_free_func_t free);
void rb_set_compare_func(struct rbtree *rb, rb_cmp_func_t func);
void rb_set_delete_func(struct rbtree *rb, rb_del_func_t func, void *cls);
/* TODO add user deep copy function */

void rb_clear(struct rbtree *rb);
int rb_copy(struct rbtree *dest, struct rbtree *src);

int rb_size(struct rbtree *rb);

int rb_insert(struct rbtree *rb, void *key, void *data);
int rb_inserti(struct rbtree *rb, int key, void *data);

int rb_delete(struct rbtree *rb, void *key);
int rb_deletei(struct rbtree *rb, int key);

struct rbnode *rb_find(struct rbtree *rb, void *key);
struct rbnode *rb_findi(struct rbtree *rb, int key);

void rb_foreach(struct rbtree *rb, void (*func)(struct rbnode*, void*), void *cls);

struct rbnode *rb_root(struct rbtree *rb);

void rb_begin(struct rbtree *rb);
struct rbnode *rb_next(struct rbtree *rb);

void *rb_node_key(struct rbnode *node);
int rb_node_keyi(struct rbnode *node);
void *rb_node_data(struct rbnode *node);

#ifdef __cplusplus
}
#endif


#endif	/* RBTREE_H_ */
