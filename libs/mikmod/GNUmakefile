src = $(wildcard drivers/*.c) \
	  $(wildcard loaders/*.c) \
	  $(wildcard mmio/*.c) \
	  $(wildcard depackers/*.c) \
	  $(wildcard playercode/*.c) \
	  $(wildcard posix/*.c)
obj = $(src:.c=.o)
alib = libmikmod.a

CFLAGS = -m32 -g -I. -Iinclude -DHAVE_CONFIG_H -DMIKMOD_STATIC `sdl-config --cflags`

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
