csrc = $(wildcard src/*.c) $(wildcard src/sdl/*.c) $(wildcard src/3dgfx/*.c)

obj = $(csrc:.c=.o)
dep = $(obj:.o=.d)
bin = game

inc = -Isrc -Isrc/sdl -Isrc/3dgfx

CFLAGS = $(arch) -pedantic -Wall -g -MMD $(inc) `sdl-config --cflags`
LDFLAGS = $(arch) -Llibs/imago -limago `sdl-config --libs` -lm

ifneq ($(shell uname -m), i386)
	arch = -m32
	sdl_ldflags = -L/usr/lib/i386-linux-gnu -lSDL-1.2
else
	sdl_ldflags = `sdl-config --libs`
endif


$(bin): $(obj) imago
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: imago
imago:
	$(MAKE) -C libs/imago

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
