#include <stdlib.h>
#include <ctype.h>
#include "util.h"

uint32_t perf_start_count, perf_interval_count;


#ifdef __WATCOMC__
int strcasecmp(const char *a, const char *b)
{
	int ca, cb;

	while(*a && *b) {
		ca = tolower(*a++);
		cb = tolower(*b++);
		if(ca != cb) return ca - cb;
	}

	if(!*a && !*b) return 0;
	if(!*a) return -1;
	if(!*b) return 1;
	return 0;
}

int strncasecmp(const char *a, const char *b, size_t n)
{
	int ca, cb;
	while(n-- > 0 && *a && *b) {
		ca = tolower(*a++);
		cb = tolower(*b++);
		if(ca != cb) return ca - cb;
	}

	if(!*a && !*b) return 0;
	if(!*a) return -1;
	if(!*b) return 1;
	return 0;
}
#endif
