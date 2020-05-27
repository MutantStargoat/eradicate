#ifndef MIKMOD_CONFIG_H_
#define MIKMOD_CONFIG_H_

#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMCMP 1
#define HAVE_MEMORY_H 1
#define HAVE_POSIX_MEMALIGN 1
#define HAVE_SETENV 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRSTR 1

#if !defined(_MSC_VER) || _MSC_VER >= 1800
#define HAVE_STDINT_H 1
#define HAVE_SNPRINTF 1
#endif

#ifdef __unix__
#define HAVE_DLFCN_H 1
#define HAVE_RTLD_GLOBAL 1
#define HAVE_FCNTL_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_PTHREAD 1
#endif

#define STDC_HEADERS 1

#ifdef __linux__
#define DRV_ALSA	1
#endif

#ifdef __FreeBSD__
#define DRV_OSS		1
#endif

#ifdef __sgi__
#define DRV_SGI		1
#endif

#ifdef _WIN32
#define DRV_DS	1
#endif

#endif	/* MIKMOD_CONFIG_H_ */
