src = $(wildcard src/*.c) \
	  $(wildcard zlib/*.c) \
	  $(wildcard libpng/*.c) \
	  $(wildcard jpeglib/*.c)
obj = $(src:.c=.o)
alib = libimago.a

CFLAGS = -m32 -fno-pie -Wno-main -g -Izlib -Ilibpng -Ijpeglib

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
