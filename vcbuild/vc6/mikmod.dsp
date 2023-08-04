# Microsoft Developer Studio Project File - Name="mikmod" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mikmod - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mikmod.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mikmod.mak" CFG="mikmod - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mikmod - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "mikmod - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mikmod - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MIKMOD_STATIC" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mikmod - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "mikmod___Win32_Debug"
# PROP BASE Intermediate_Dir "mikmod___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "mikmod___Win32_Debug"
# PROP Intermediate_Dir "mikmod___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MIKMOD_STATIC" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
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

# Name "mikmod - Win32 Release"
# Name "mikmod - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\drivers\drv_nos.c
# End Source File
# Begin Source File

SOURCE=.\drivers\drv_win.c
# End Source File
# Begin Source File

SOURCE=.\loaders\load_it.c
# End Source File
# Begin Source File

SOURCE=.\loaders\load_mod.c
# End Source File
# Begin Source File

SOURCE=.\loaders\load_s3m.c
# End Source File
# Begin Source File

SOURCE=.\loaders\load_xm.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mdreg.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mdriver.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mdulaw.c
# End Source File
# Begin Source File

SOURCE=.\include\mikmod.h
# End Source File
# Begin Source File

SOURCE=.\include\mikmod_ctype.h
# End Source File
# Begin Source File

SOURCE=.\include\mikmod_internals.h
# End Source File
# Begin Source File

SOURCE=.\playercode\mloader.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mlreg.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mlutil.c
# End Source File
# Begin Source File

SOURCE=.\mmio\mmalloc.c
# End Source File
# Begin Source File

SOURCE=.\depackers\mmcmp.c
# End Source File
# Begin Source File

SOURCE=.\mmio\mmerror.c
# End Source File
# Begin Source File

SOURCE=.\mmio\mmio.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mplayer.c
# End Source File
# Begin Source File

SOURCE=.\playercode\munitrk.c
# End Source File
# Begin Source File

SOURCE=.\playercode\mwav.c
# End Source File
# Begin Source File

SOURCE=.\playercode\npertab.c
# End Source File
# Begin Source File

SOURCE=.\depackers\pp20.c
# End Source File
# Begin Source File

SOURCE=.\depackers\s404.c
# End Source File
# Begin Source File

SOURCE=.\playercode\sloader.c
# End Source File
# Begin Source File

SOURCE=.\posix\strcasecmp.c
# End Source File
# Begin Source File

SOURCE=.\playercode\virtch.c
# End Source File
# Begin Source File

SOURCE=.\playercode\virtch2.c
# End Source File
# Begin Source File

SOURCE=.\playercode\virtch_common.c
# End Source File
# Begin Source File

SOURCE=.\depackers\xpk.c
# End Source File
# End Group
# End Target
# End Project
