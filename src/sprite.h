#ifndef SPRITE_H_
#define SPRITE_H_

enum {
	SOP_END,
	SOP_ENDL,
	SOP_SKIP,
	SOP_RESVD,
	SOP_COPY
};

#define SOP_OP(op)	((op) & 0xff)
#define SOP_LEN(op)	((op) >> 16)

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

#endif	/* SPRITE_H_ */
