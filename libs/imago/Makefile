!ifdef __UNIX__
libpng = libpng/png.obj libpng/pngerror.obj libpng/pngget.obj libpng/pngmem.obj &
	libpng/pngpread.obj libpng/pngread.obj libpng/pngrio.obj libpng/pngrtran.obj &
	libpng/pngrutil.obj libpng/pngset.obj libpng/pngtrans.obj libpng/pngwio.obj &
	libpng/pngwrite.obj libpng/pngwtran.obj libpng/pngwutil.obj
zlib = zlib/adler32.obj zlib/compress.obj zlib/crc32.obj zlib/deflate.obj &
	zlib/gzio.obj zlib/infback.obj zlib/inffast.obj zlib/inflate.obj &
	zlib/inftrees.obj zlib/trees.obj zlib/uncompr.obj zlib/zutil.obj
jpeglib = jpeglib/jcapimin.obj jpeglib/jcapistd.obj jpeglib/jccoefct.obj &
	jpeglib/jccolor.obj jpeglib/jcdctmgr.obj jpeglib/jchuff.obj jpeglib/jcinit.obj &
	jpeglib/jcmainct.obj jpeglib/jcmarker.obj jpeglib/jcmaster.obj jpeglib/jcomapi.obj &
	jpeglib/jcparam.obj jpeglib/jcphuff.obj jpeglib/jcprepct.obj jpeglib/jcsample.obj &
	jpeglib/jctrans.obj jpeglib/jdapimin.obj jpeglib/jdapistd.obj jpeglib/jdatadst.obj &
	jpeglib/jdatasrc.obj jpeglib/jdcoefct.obj jpeglib/jdcolor.obj jpeglib/jddctmgr.obj &
	jpeglib/jdhuff.obj jpeglib/jdinput.obj jpeglib/jdmainct.obj jpeglib/jdmarker.obj &
	jpeglib/jdmaster.obj jpeglib/jdmerge.obj jpeglib/jdphuff.obj jpeglib/jdpostct.obj &
	jpeglib/jdsample.obj jpeglib/jdtrans.obj jpeglib/jerror.obj jpeglib/jfdctflt.obj &
	jpeglib/jfdctfst.obj jpeglib/jfdctint.obj jpeglib/jidctflt.obj jpeglib/jidctfst.obj &
	jpeglib/jidctint.obj jpeglib/jidctred.obj jpeglib/jmemmgr.obj jpeglib/jmemnobs.obj &
	jpeglib/jquant1.obj jpeglib/jquant2.obj jpeglib/jutils.obj
obj = src/conv.obj src/filejpeg.obj src/filepng.obj src/fileppm.obj src/filergbe.obj &
	src/filetga.obj src/ftmodule.obj src/imago2.obj src/imago_gl.obj src/modules.obj &
	$(libpng) $(zlib) $(jpeglib)
!else
libpng = libpng\png.obj libpng\pngerror.obj libpng\pngget.obj libpng\pngmem.obj &
	libpng\pngpread.obj libpng\pngread.obj libpng\pngrio.obj libpng\pngrtran.obj &
	libpng\pngrutil.obj libpng\pngset.obj libpng\pngtrans.obj libpng\pngwio.obj &
	libpng\pngwrite.obj libpng\pngwtran.obj libpng\pngwutil.obj
zlib = zlib\adler32.obj zlib\compress.obj zlib\crc32.obj zlib\deflate.obj &
	zlib\gzio.obj zlib\infback.obj zlib\inffast.obj zlib\inflate.obj &
	zlib\inftrees.obj zlib\trees.obj zlib\uncompr.obj zlib\zutil.obj
jpeglib = jpeglib\jcapimin.obj jpeglib\jcapistd.obj jpeglib\jccoefct.obj &
	jpeglib\jccolor.obj jpeglib\jcdctmgr.obj jpeglib\jchuff.obj jpeglib\jcinit.obj &
	jpeglib\jcmainct.obj jpeglib\jcmarker.obj jpeglib\jcmaster.obj jpeglib\jcomapi.obj &
	jpeglib\jcparam.obj jpeglib\jcphuff.obj jpeglib\jcprepct.obj jpeglib\jcsample.obj &
	jpeglib\jctrans.obj jpeglib\jdapimin.obj jpeglib\jdapistd.obj jpeglib\jdatadst.obj &
	jpeglib\jdatasrc.obj jpeglib\jdcoefct.obj jpeglib\jdcolor.obj jpeglib\jddctmgr.obj &
	jpeglib\jdhuff.obj jpeglib\jdinput.obj jpeglib\jdmainct.obj jpeglib\jdmarker.obj &
	jpeglib\jdmaster.obj jpeglib\jdmerge.obj jpeglib\jdphuff.obj jpeglib\jdpostct.obj &
	jpeglib\jdsample.obj jpeglib\jdtrans.obj jpeglib\jerror.obj jpeglib\jfdctflt.obj &
	jpeglib\jfdctfst.obj jpeglib\jfdctint.obj jpeglib\jidctflt.obj jpeglib\jidctfst.obj &
	jpeglib\jidctint.obj jpeglib\jidctred.obj jpeglib\jmemmgr.obj jpeglib\jmemnobs.obj &
	jpeglib\jquant1.obj jpeglib\jquant2.obj jpeglib\jutils.obj
obj = src\conv.obj src\filejpeg.obj src\filepng.obj src\fileppm.obj src\filergbe.obj &
	src\filetga.obj src\ftmodule.obj src\imago2.obj src\imago_gl.obj src\modules.obj &
	$(libpng) $(zlib) $(jpeglib)
!endif

alib = imago.lib

opt = -5 -fp5 -otexan
dbg = -d1
def = -DPNG_NO_SNPRINTF

CC = wcc386
CFLAGS = $(dbg) $(opt) $(def) -zq -bt=dos -Ilibpng -Izlib -Ijpeglib

$(alib): $(obj)
	%write objects.lbc $(obj)
	wlib -b -n $@ @objects

.c: src;libpng;jpeglib;zlib

.c.obj: .autodepend
	$(CC) -fo=$@ $(CFLAGS) $[*

!ifdef __UNIX__
clean: .symbolic
	rm -f $(obj)
	rm -f $(alib)
!else
clean: .symbolic
	del src\*.obj
	del zlib\*.obj
	del libpng\*.obj
	del jpeglib\*.obj
	del objects.lbc
	del $(alib)
!endif
