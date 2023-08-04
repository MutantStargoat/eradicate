src = $(wildcard src/*.c) \
	  $(wildcard zlib/*.c) \
	  $(wildcard libpng/*.c) \
	  $(wildcard jpeglib/*.c)
obj = $(src:.c=.o)
alib = ../unix/libimago.a

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	alib = ../w32/libimago.a
endif

CC = gcc
CFLAGS = $(arch) -fno-pie -Wno-main -g -O3 -Izlib -Ilibpng -Ijpeglib

#ifeq ($(shell uname -m), x86_64)
#	arch = -m32
#endif

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
