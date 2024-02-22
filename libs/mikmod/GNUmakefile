src = $(wildcard drivers/*.c) \
	  $(wildcard loaders/*.c) \
	  $(wildcard mmio/*.c) \
	  $(wildcard depack/*.c) \
	  $(wildcard player/*.c) \
	  $(wildcard posix/*.c)
obj = $(src:.c=.o)
alib = ../unix/libmikmod.a

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	alib = ../w32/libmikmod.a
endif

CFLAGS = -fno-pie -O3 -I. -Iinclude -DHAVE_CONFIG_H -DMIKMOD_STATIC

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
