#ifndef UTIL_H_
#define UTIL_H_

#include "inttypes.h"

#ifdef __GNUC__
#define INLINE __inline
#define PACKED __attribute__((packed))

#elif defined(__WATCOMC__)
#define INLINE __inline
#define PACKED

#else
#define INLINE
#define PACKED
#endif

/* fast conversion of double -> 32bit int
 * for details see:
 *  - http://chrishecker.com/images/f/fb/Gdmfp.pdf
 *  - http://stereopsis.com/FPU.html#convert
 */
static INLINE int32_t cround64(double val)
{
	val += 6755399441055744.0;
	return *(int32_t*)&val;
}

extern uint32_t perf_start_count, perf_interval_count;

#ifdef __WATCOMC__
#ifdef USE_MMX
void memcpy64(void *dest, void *src, int count);
#pragma aux memcpy64 = \
	"cploop:" \
	"movq mm0, [edx]" \
	"movq [ebx], mm0" \
	"add edx, 8" \
	"add ebx, 8" \
	"dec ecx" \
	"jnz cploop" \
	"emms" \
	parm[ebx][edx][ecx] \
	modify[8087];
#else
#define memcpy64(dest, src, count)	memcpy(dest, src, (count) << 3)
#endif

void perf_start(void);
#pragma aux perf_start = \
	"xor eax, eax" \
	"cpuid" \
	"rdtsc" \
	"mov [perf_start_count], eax" \
	modify[eax ebx ecx edx];

void perf_end(void);
#pragma aux perf_end = \
	"xor eax, eax" \
	"cpuid" \
	"rdtsc" \
	"sub eax, [perf_start_count]" \
	"mov [perf_interval_count], eax" \
	modify [eax ebx ecx edx];

void debug_break(void);
#pragma aux debug_break = "int 3";
#endif

#ifdef __GNUC__
#ifdef USE_MMX
#define memcpy64(dest, src, count) asm volatile ( \
	"0:\n\t" \
	"movq (%1), %%mm0\n\t" \
	"movq %%mm0, (%0)\n\t" \
	"add $8, %1\n\t" \
	"add $8, %0\n\t" \
	"dec %2\n\t" \
	"jnz 0b\n\t" \
	"emms\n\t" \
	:: "r"(dest), "r"(src), "r"(count) \
	: "%mm0")
#else
#define memcpy64(dest, src, count)	memcpy(dest, src, (count) << 3)
#endif

#define perf_start()  asm volatile ( \
	"xor %%eax, %%eax\n" \
	"cpuid\n" \
	"rdtsc\n" \
	"mov %%eax, %0\n" \
	: "=m"(perf_start_count) \
	:: "%eax", "%ebx", "%ecx", "%edx")

#define perf_end() asm volatile ( \
	"xor %%eax, %%eax\n" \
	"cpuid\n" \
	"rdtsc\n" \
	"sub %1, %%eax\n" \
	"mov %%eax, %0\n" \
	: "=m"(perf_interval_count) \
	: "m"(perf_start_count) \
	: "%eax", "%ebx", "%ecx", "%edx")

#define debug_break() \
	asm volatile ("int $3")
#endif

#ifdef _MSC_VER
#define perf_start() \
	do { \
		__asm { \
			xor eax, eax \
			cpuid \
			rdtsc \
			mov [perf_start_count], eax \
		} \
	} while(0)

#define perf_end() \
	do { \
		__asm { \
			xor eax, eax \
			cpuid \
			rdtsc \
			sub eax, [perf_start_count] \
			mov [perf_interval_count], eax \
		} \
	} while(0)

#define debug_break() \
	do { \
		__asm { int 3 } \
	} while(0)
#endif

#endif	/* UTIL_H_ */
