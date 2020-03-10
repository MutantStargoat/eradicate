csrc = $(wildcard src/*.c) $(wildcard src/sdl/*.c) $(wildcard src/3dgfx/*.c)
asmsrc = $(wildcard src/*.asm)

obj = $(csrc:.c=.o) $(asmsrc:.asm=.o)
dep = $(obj:.o=.d)
bin = game

def = -DUSE_MMX
inc = -Isrc -Isrc/sdl -Isrc/3dgfx -Ilibs/imago/src
warn = -pedantic -Wall

CFLAGS = $(arch) $(warn) -g -MMD $(def) $(inc) `sdl-config --cflags`
LDFLAGS = $(arch) -Llibs/imago -limago $(sdl_ldflags) -lm

ifneq ($(shell uname -m), i386)
	arch = -m32
	sdl_ldflags = -L/usr/lib/i386-linux-gnu -lSDL-1.2
else
	sdl_ldflags = `sdl-config --libs`
endif

.PHONY: all
all: $(bin)

$(bin): $(obj) imago
	$(CC) -o $@ $(obj) $(LDFLAGS)

%.o: %.asm
	nasm -f elf -o $@ $<

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

.PHONY: data
data:
	@tools/procdata
