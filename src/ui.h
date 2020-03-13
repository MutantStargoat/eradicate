#ifndef UI_H_
#define UI_H_

enum {
	UI_UNKNOWN,
	UI_BUTTONBOX,
	UI_CHECKBOX,
	UI_ITEMLIST
};

struct ui_base {
	int type;
	int x, y;
	char *text;
	int focus;
	struct ui_base *next, *prev;

	void (*draw)();
	void (*keypress)();
	void (*free)();
};

struct ui_bnbox {
	struct ui_base w;
	char *text2;
	int sel;
};

struct ui_ckbox {
	struct ui_base w;
	int val;
};

struct ui_list_item {
	char *name;
	void *data;
};

struct ui_list {
	struct ui_base w;
	struct ui_list_item *items;
	int num_items, max_items, sel;
};

struct ui_bnbox *ui_bnbox(const char *tx1, const char *tx2);
struct ui_ckbox *ui_ckbox(const char *text, int chk);
struct ui_list *ui_list(const char *text);

void ui_free(void *w);

void ui_move(void *w, int x, int y);
int ui_set_text(void *w, const char *text);

void ui_set_focus(void *w, int focus);

void ui_draw(void *w);
void ui_keypress(void *w, int key);

int ui_ckbox_state(struct ui_ckbox *w);
void ui_ckbox_set(struct ui_ckbox *w, int val);

void ui_bnbox_select(struct ui_bnbox *w, int sel);
void ui_bnbox_next(struct ui_list *w);
void ui_bnbox_prev(struct ui_list *w);

int ui_list_append(struct ui_list *w, const char *name, void *udata);
void ui_list_select(struct ui_list *w, int sel);
void ui_list_next(struct ui_list *w);
void ui_list_prev(struct ui_list *w);

#endif	/* UI_H_ */