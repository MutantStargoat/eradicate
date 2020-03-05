!ifdef __UNIX__
dosobj = src/dos/main.obj src/dos/gfx.obj src/dos/timer.obj src/dos/watdpmi.obj &
		 src/dos/vbe.obj src/dos/vga.obj src/dos/keyb.obj src/dos/mouse.obj &
		 src/dos/logger.obj
scrobj = src/introscr.obj src/menuscr.obj
gameobj = src/game.obj src/util.obj src/gfxutil.obj src/dynarr.obj src/rbtree.obj
gfxobj = src/3dgfx/3dgfx.obj src/3dgfx/mesh.obj src/3dgfx/meshload.obj &
		src/3dgfx/polyfill.obj src/3dgfx/polyclip.obj
incpath = -Isrc -Isrc/dos -Ilibs/imago/src
libpath = libpath libs/imago
!else
dosobj = src\dos\main.obj src\dos\gfx.obj src\dos\timer.obj src\dos\watdpmi.obj &
		 src\dos\vbe.obj src\dos\vga.obj src\dos\keyb.obj src\dos\mouse.obj &
		 src\dos\logger.obj
scrobj = src\introscr.obj src\menuscr.obj
gameobj = src\game.obj src\util.obj src\gfxutil.obj src\dynarr.obj src\rbtree.obj
gfxobj = src\3dgfx\3dgfx.obj src\3dgfx\mesh.obj src\3dgfx\meshload.obj &
		src\3dgfx\polyfill.obj src\3dgfx\polyclip.obj
incpath = -Isrc -Isrc\dos -Ilibs\imago\src
libpath = libpath libs\imago
!endif

obj = $(dosobj) $(gameobj) $(gfxobj) $(scrobj)
bin = game.exe

def = -dM_PI=3.141592653589793
libs = imago.lib

CC = wcc386
LD = wlink
CFLAGS = -d3 -5 -fp5 -otebmileran $(def) -s -zq -bt=dos $(incpath)
LDFLAGS = option map $(libpath) library { $(libs) }

$(bin): $(obj)
	%write objects.lnk $(obj)
	%write ldflags.lnk $(LDFLAGS)
	$(LD) debug all name $@ system dos4g file { @objects } @ldflags

.c: src;src/dos;src/3dgfx
.asm: src;src/dos;src/3dgfx

.c.obj: .autodepend
	$(CC) -fo=$@ $(CFLAGS) $[*

.asm.obj:
	nasm -f obj -o $@ $[*.asm

!ifdef __UNIX__
clean: .symbolic
	rm -f $(obj)
	rm -f $(bin)
!else
clean: .symbolic
	del src\*.obj
	del src\dos\*.obj
	del *.lnk
	del $(bin)
!endif
