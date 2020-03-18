/* dynarr - dynamic resizable C array data structure
 * author: John Tsiombikas <nuclear@member.fsf.org>
 * license: public domain
 */
#ifndef DYNARR_H_
#define DYNARR_H_

/* usage example:
 * -------------
 * int *arr = dynarr_alloc(0, sizeof *arr);
 *
 * int x = 10;
 * arr = dynarr_push(arr, &x);
 * x = 5;
 * arr = dynarr_push(arr, &x);
 * x = 42;
 * arr = dynarr_push(arr, &x);
 *
 * for(i=0; i<dynarr_size(arr); i++) {
 *     printf("%d\n", arr[i]);
 *  }
 *  dynarr_free(arr);
 */

void *dynarr_alloc(int elem, int szelem);
void dynarr_free(void *da);
void *dynarr_resize(void *da, int elem);

/* dynarr_empty returns non-zero if the array is empty
 * Complexity: O(1) */
int dynarr_empty(void *da);
/* dynarr_size returns the number of elements in the array
 * Complexity: O(1) */
int dynarr_size(void *da);

void *dynarr_clear(void *da);

/* stack semantics */
void *dynarr_push(void *da, void *item);
void *dynarr_pop(void *da);

/* Finalize the array. No more resizing is possible after this call.
 * Use free() instead of dynarr_free() to deallocate a finalized array.
 * Returns pointer to the finalized array.
 * dynarr_finalize can't fail.
 * Complexity: O(n)
 */
void *dynarr_finalize(void *da);

/* helper macros */
#define DYNARR_RESIZE(da, n) \
	do { (da) = dynarr_resize((da), (n)); } while(0)
#define DYNARR_CLEAR(da) \
	do { (da) = dynarr_clear(da); } while(0)
#define DYNARR_PUSH(da, item) \
	do { (da) = dynarr_push((da), (item)); } while(0)
#define DYNARR_POP(da) \
	do { (da) = dynarr_pop(da); } while(0)

/* utility macros to push characters to a string. assumes and maintains
 * the invariant that the last element is always a zero
 */
#define DYNARR_STRPUSH(da, c) \
	do { \
		char cnull = 0, ch = (char)(c); \
		(da) = dynarr_pop(da); \
		(da) = dynarr_push((da), &ch); \
		(da) = dynarr_push((da), &cnull); \
	} while(0)

#define DYNARR_STRPOP(da) \
	do { \
		char cnull = 0; \
		(da) = dynarr_pop(da); \
		(da) = dynarr_pop(da); \
		(da) = dynarr_push((da), &cnull); \
	} while(0)


#endif	/* DYNARR_H_ */
