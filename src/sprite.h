#ifndef SPRITE_H_
#define SPRITE_H_

enum {
	SOP_END,
	SOP_ENDL,
	SOP_SKIP,
	SOP_RESVD,
	SOP_COPY
};

struct sprite_op {
	unsigned char op;
	unsigned short size;
	void *data;
};

struct sprite {
	struct sprite_op *ops;
	int num_ops;
};

struct sprites {
	int width, height, bpp;

	struct sprite *sprites;
	int num_sprites;
};

void destroy_sprites(struct sprites *ss);

int load_sprites(struct sprites *ss, const char *fname);

void draw_sprite(void *dest, int fbpitch, struct sprites *ss, int idx);

#endif	/* SPRITE_H_ */
