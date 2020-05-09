csrc = $(wildcard src/*.c) $(wildcard src/sdl/*.c) $(wildcard src/3dgfx/*.c)
asmsrc = $(wildcard src/*.asm)

obj = $(csrc:.c=.o) $(asmsrc:.asm=.o)
dep = $(obj:.o=.d)
bin = game

inc = -Isrc -Isrc/sdl -Isrc/3dgfx -Ilibs/imago/src -Ilibs/cgmath/src \
	  -Ilibs/mikmod/include
warn = -pedantic -Wall

CFLAGS = $(arch) $(warn) -O3 -ffast-math -g -MMD $(def) $(inc) `sdl-config --cflags`
LDFLAGS = $(arch) -Llibs/imago -limago -Llibs/mikmod -lmikmod $(sdl_ldflags) -lm

ifneq ($(shell uname -m), i386)
	arch = -m32
	sdl_ldflags = -L/usr/lib/i386-linux-gnu -lSDL-1.2
else
	sdl_ldflags = `sdl-config --libs`
endif

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(csrc:.c=.w32.o)
	dep	= $(obj:.o=.d)

	bin = game_win32.exe

	LDFLAGS += -lmingw32 -lSDL -lSDLmain -lwinmm -mconsole
else
	def = -DUSE_MMX
endif


.PHONY: all
all: $(bin)

$(bin): $(obj) imago mikmod
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.o: %.asm
	nasm -f elf -o $@ $<

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: imago
imago:
	$(MAKE) -C libs/imago

.PHONY: mikmod
mikmod:
	$(MAKE) -C libs/mikmod

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: data
data:
	@tools/procdata

.PHONY: crosswin
crosswin:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw
