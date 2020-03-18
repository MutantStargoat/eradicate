!ifdef __UNIX__
dosobj = src/dos/main.obj src/dos/gfx.obj src/dos/timer.obj src/dos/watdpmi.obj &
	src/dos/vbe.obj src/dos/vga.obj src/dos/keyb.obj src/dos/mouse.obj &
	src/dos/logger.obj
scrobj = src/introscr.obj src/intro_s.obj src/menuscr.obj src/racescr.obj &
	src/optscr.obj
gameobj = src/game.obj src/util.obj src/gfxutil.obj src/dynarr.obj &
	src/rbtree.obj src/treestor.obj src/ts_text.obj src/options.obj src/fonts.obj &
	src/ui.obj
gfxobj = src/3dgfx/3dgfx.obj src/3dgfx/mesh.obj src/3dgfx/meshload.obj &
	src/3dgfx/polyfill.obj src/3dgfx/polyclip.obj src/sprite.obj

incpath = -Isrc -Isrc/dos -Ilibs/imago/src
libpath = libpath libs/imago

!else

dosobj = src\dos\main.obj src\dos\gfx.obj src\dos\timer.obj src\dos\watdpmi.obj &
	src\dos\vbe.obj src\dos\vga.obj src\dos\keyb.obj src\dos\mouse.obj &
	src\dos\logger.obj
scrobj = src\introscr.obj src\intro_s.obj src\menuscr.obj src\racescr.obj &
	src\optscr.obj
gameobj = src\game.obj src\util.obj src\gfxutil.obj src\dynarr.obj &
	src\rbtree.obj src\treestor.obj src\ts_text.obj src\options.obj src\fonts.obj &
	src\ui.obj
gfxobj = src\3dgfx\3dgfx.obj src\3dgfx\mesh.obj src\3dgfx\meshload.obj &
	src\3dgfx\polyfill.obj src\3dgfx\polyclip.obj src\sprite.obj

incpath = -Isrc -Isrc\dos -Ilibs\imago\src
libpath = libpath libs\imago
!endif

obj = $(dosobj) $(gameobj) $(gfxobj) $(scrobj)
bin = game.exe

def = -dM_PI=3.141592653589793 -dUSE_MMX
libs = imago.lib

CC = wcc386
LD = wlink
CFLAGS = -d3 -5 -fp5 -otexan $(def) -s -zq -bt=dos $(incpath)
LDFLAGS = option map $(libpath) library { $(libs) }

$(bin): cflags.occ $(obj) libs/imago/imago.lib
	%write objects.lnk $(obj)
	%write ldflags.lnk $(LDFLAGS)
	$(LD) debug all name $@ system dos4g file { @objects } @ldflags

.c: src;src/dos;src/3dgfx
.asm: src;src/dos;src/3dgfx

cflags.occ: Makefile
	%write $@ $(CFLAGS)

.c.obj: .autodepend
	$(CC) -fo=$@ @cflags.occ $[*

.asm.obj:
	nasm -f obj -o $@ $[*.asm

!ifdef __UNIX__
clean: .symbolic
	rm -f $(obj)
	rm -f $(bin)
	rm -f cflags.occ *.lnk
!else
clean: .symbolic
	del src\*.obj
	del src\dos\*.obj
	del src\3dgfx\*.obj
	del *.lnk
	del cflags.occ
	del $(bin)
!endif
