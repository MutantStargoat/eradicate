# Microsoft Developer Studio Project File - Name="imago" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=imago - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "imago.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "imago.mak" CFG="imago - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "imago - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "imago - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "imago - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W2 /GX /O2 /I "zlib" /I "libpng" /I "jpeglib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "imago - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "imago___Win32_Debug"
# PROP BASE Intermediate_Dir "imago___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "imago___Win32_Debug"
# PROP Intermediate_Dir "imago___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W2 /Gm /GX /ZI /Od /I "zlib" /I "libpng" /I "jpeglib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "imago - Win32 Release"
# Name "imago - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=.\src\conv.c
# End Source File
# Begin Source File

SOURCE=.\src\filejpeg.c
# End Source File
# Begin Source File

SOURCE=.\src\filepng.c
# End Source File
# Begin Source File

SOURCE=.\src\fileppm.c
# End Source File
# Begin Source File

SOURCE=.\src\filergbe.c
# End Source File
# Begin Source File

SOURCE=.\src\filetga.c
# End Source File
# Begin Source File

SOURCE=.\src\ftmodule.c
# End Source File
# Begin Source File

SOURCE=.\src\ftmodule.h
# End Source File
# Begin Source File

SOURCE=.\src\imago2.c
# End Source File
# Begin Source File

SOURCE=.\src\imago2.h
# End Source File
# Begin Source File

SOURCE=.\src\imago_gl.c
# End Source File
# Begin Source File

SOURCE=.\src\inttypes.h
# End Source File
# Begin Source File

SOURCE=.\src\modules.c
# End Source File
# End Group
# Begin Group "jpeglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\jpeglib\cderror.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcapimin.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcapistd.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jccoefct.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jccolor.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jchuff.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jchuff.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcinit.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcmainct.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcmarker.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcmaster.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcomapi.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcparam.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcphuff.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcprepct.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jcsample.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jctrans.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdapimin.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdapistd.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdatadst.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdcolor.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdct.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdhuff.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdinput.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdmainct.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdmarker.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdmaster.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdmerge.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdphuff.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdpostct.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdsample.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jdtrans.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jerror.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jerror.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jfdctint.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jidctflt.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jidctfst.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jidctint.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jidctred.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jinclude.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jquant1.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jquant2.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jutils.c
# End Source File
# Begin Source File

SOURCE=.\jpeglib\jversion.h
# End Source File
# End Group
# Begin Group "libpng"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libpng\png.c
# End Source File
# Begin Source File

SOURCE=.\libpng\png.h
# End Source File
# Begin Source File

SOURCE=.\libpng\pngconf.h
# End Source File
# Begin Source File

SOURCE=.\libpng\pngerror.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngget.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngmem.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngpread.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngread.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngrio.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngrtran.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngrutil.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngset.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngtrans.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngwio.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngwrite.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngwtran.c
# End Source File
# Begin Source File

SOURCE=.\libpng\pngwutil.c
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=.\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=.\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.h
# End Source File
# End Group
# End Target
# End Project
