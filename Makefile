!ifdef __UNIX__
dosobj = src/dos/main.obj src/dos/gfx.obj src/dos/timer.obj src/dos/watdpmi.obj &
		 src/dos/vbe.obj src/dos/vga.obj src/dos/keyb.obj src/dos/mouse.obj &
		 src/dos/logger.obj
gameobj = src/game.obj src/util.obj src/gfxutil.obj src/menuscr.obj
incpath = -Isrc -Isrc/dos -Ilibs/imago/src
libpath = libpath libs/imago
!else
dosobj = src\dos\main.obj src\dos\gfx.obj src\dos\timer.obj src\dos\watdpmi.obj &
		 src\dos\vbe.obj src\dos\vga.obj src\dos\keyb.obj src\dos\mouse.obj &
		 src\dos\logger.obj
gameobj = src\game.obj src\util.obj src\gfxutil.obj src\menuscr.obj
incpath = -Isrc -Isrc\dos -Ilibs\imago\src
libpath = libpath libs\imago
!endif

obj = $(dosobj) $(gameobj)
bin = game.exe

libs = imago.lib

CC = wcc386
LD = wlink
CFLAGS = -d3 -5 -fp5 -otebmileran -s -zq -bt=dos $(incpath)
LDFLAGS = option map $(libpath) library { $(libs) }

$(bin): $(obj)
	%write objects.lnk $(obj)
	%write ldflags.lnk $(LDFLAGS)
	$(LD) debug all name $@ system dos4g file { @objects } @ldflags

.c: src;src/dos
.asm: src;src/dos

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
