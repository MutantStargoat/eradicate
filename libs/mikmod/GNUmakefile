src = $(wildcard drivers/*.c) \
	  $(wildcard loaders/*.c) \
	  $(wildcard mmio/*.c) \
	  $(wildcard depackers/*.c) \
	  $(wildcard playercode/*.c) \
	  $(wildcard posix/*.c)
obj = $(src:.c=.o)
alib = libmikmod.a

CC = gcc
CFLAGS = $(arch) -fno-pie -g -O3 -I. -Iinclude -DHAVE_CONFIG_H -DMIKMOD_STATIC `sdl-config --cflags`

#ifeq ($(shell uname -m), x86_64)
#	arch = -m32
#endif

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
