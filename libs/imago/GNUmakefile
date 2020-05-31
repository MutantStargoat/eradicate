src = $(wildcard src/*.c) \
	  $(wildcard zlib/*.c) \
	  $(wildcard libpng/*.c) \
	  $(wildcard jpeglib/*.c)
obj = $(src:.c=.o)
alib = libimago.a

CC = gcc
CFLAGS = $(arch) -fno-pie -Wno-main -g -Izlib -Ilibpng -Ijpeglib

ifeq ($(shell uname -m), x86_64)
	arch = -m32
endif

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
