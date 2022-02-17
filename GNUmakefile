# ---- build options ----
# renderer = <software|opengl>
renderer = software
# -----------------------

csrc = $(wildcard src/*.c) $(wildcard src/nondos/*.c) $(wildcard src/glut/*.c) \
	   $(wildcard src/3dgfx/*.c)
#asmsrc = $(wildcard src/*.asm)

obj = $(csrc:.c=.o)# $(asmsrc:.asm=.o)
dep = $(obj:.o=.d)
bin = game

inc = -Isrc -Isrc/nondos -Isrc/glut -Isrc/3dgfx -Ilibs/imago/src -Ilibs/cgmath/src \
	  -Ilibs/mikmod/include
def = -DMINIGLUT_USE_LIBC -DMIKMOD_STATIC
warn = -pedantic -Wall
opt = -O3 -ffast-math
dbg = -g

CC = gcc
CFLAGS = $(arch) $(warn) $(opt) -fno-pie -fno-strict-aliasing $(dbg) -MMD \
		 $(def) $(inc)
LDFLAGS = $(arch) -no-pie -Llibs/imago -limago -Llibs/mikmod \
		  -lmikmod $(sndlib_$(sys)) -lm

cpu ?= $(shell uname -m | sed 's/i.86/i386/; s/arm.*/arm/; s/aarch.*/arm/')

ifeq ($(cpu), i386)
	def += -DUSE_MMX
else
	ifeq ($(cpu), x86_64)
		#def += -DUSE_MMX
		#arch = -m32
	else
		ifeq ($(cpu), arm)
			asmsrc =
		else
			asmsrc =
			def += -DBUILD_BIGENDIAN
		endif
	endif
endif

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/; s/IRIX.*/IRIX/')
ifeq ($(sys), mingw)
	obj = $(csrc:.c=.w32.o) $(asmsrc:.asm=.o)
	dep	= $(csrc:.c=.d)

	bin = game_win32.exe

	LDFLAGS += -static-libgcc -lmingw32 -mconsole -ldsound -lgdi32 -lwinmm \
			   -lopengl32
else
	LDFLAGS += -lGL -lX11 -lpthread
endif

sndlib_Linux = -lasound
sndlib_IRIX = -laudio

ifeq ($(renderer), opengl)
	def += -DBUILD_OPENGL
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

.PHONY: libs
libs: imago mikmod

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

.PHONY: clean-libs
clean-libs:
	$(MAKE) -C libs/imago clean
	$(MAKE) -C libs/mikmod clean

.PHONY: data
data:
	@tools/procdata

.PHONY: crosswin
crosswin:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw
