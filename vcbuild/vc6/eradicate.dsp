# Microsoft Developer Studio Project File - Name="eradicate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=eradicate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "eradicate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eradicate.mak" CFG="eradicate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eradicate - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "eradicate - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "eradicate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W2 /GX /O2 /I "src" /I "src\glut" /I "libs" /I "libs\cgmath\src" /I "libs\imago\src" /I "libs\mikmod\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D M_PI=3.141592653 /D "MIKMOD_STATIC" /D "FREEGLUT_STATIC" /D "MINIGLUT_WINMAIN" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "eradicate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W2 /Gm /GX /ZI /Od /I "src" /I "src\nondos" /I "src\ddraw" /I "libs" /I "libs\cgmath\src" /I "libs\imago\src" /I "libs\mikmod\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D M_PI=3.141592653 /D "MIKMOD_STATIC" /D "FREEGLUT_STATIC" /D "MINIGLUT_WINMAIN" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "eradicate - Win32 Release"
# Name "eradicate - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Group "nondos"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\nondos\audio.c
# End Source File
# Begin Source File

SOURCE=.\src\nondos\gfx.h
# End Source File
# Begin Source File

SOURCE=.\src\nondos\w32_dirent.c
# End Source File
# Begin Source File

SOURCE=.\src\nondos\w32_dirent.h
# End Source File
# End Group
# Begin Group "3dgfx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\3dgfx\3dgfx.c
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\3dgfx.h
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\mesh.c
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\mesh.h
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\meshload.c
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\polyclip.c
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\polyclip.h
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\polyfill.c
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\polyfill.h
# End Source File
# Begin Source File

SOURCE=.\src\3dgfx\polytmpl.h
# End Source File
# End Group
# Begin Group "glut"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\glut\main.c

!IF  "$(CFG)" == "eradicate - Win32 Release"

!ELSEIF  "$(CFG)" == "eradicate - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\glut\miniglut.c

!IF  "$(CFG)" == "eradicate - Win32 Release"

!ELSEIF  "$(CFG)" == "eradicate - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\glut\miniglut.h

!IF  "$(CFG)" == "eradicate - Win32 Release"

!ELSEIF  "$(CFG)" == "eradicate - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\glut\sprgl.c

!IF  "$(CFG)" == "eradicate - Win32 Release"

!ELSEIF  "$(CFG)" == "eradicate - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "ddraw"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ddraw\main.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\audio.h
# End Source File
# Begin Source File

SOURCE=.\src\camera.c
# End Source File
# Begin Source File

SOURCE=.\src\camera.h
# End Source File
# Begin Source File

SOURCE=.\src\curve.c
# End Source File
# Begin Source File

SOURCE=.\src\curve.h
# End Source File
# Begin Source File

SOURCE=.\src\dynarr.c
# End Source File
# Begin Source File

SOURCE=.\src\dynarr.h
# End Source File
# Begin Source File

SOURCE=.\src\fonts.c
# End Source File
# Begin Source File

SOURCE=.\src\fonts.h
# End Source File
# Begin Source File

SOURCE=.\src\game.c
# End Source File
# Begin Source File

SOURCE=.\src\game.h
# End Source File
# Begin Source File

SOURCE=.\src\gfxutil.c
# End Source File
# Begin Source File

SOURCE=.\src\gfxutil.h
# End Source File
# Begin Source File

SOURCE=.\src\image.c
# End Source File
# Begin Source File

SOURCE=.\src\image.h
# End Source File
# Begin Source File

SOURCE=.\src\input.c
# End Source File
# Begin Source File

SOURCE=.\src\input.h
# End Source File
# Begin Source File

SOURCE=.\src\introscr.c
# End Source File
# Begin Source File

SOURCE=.\src\joy.c
# End Source File
# Begin Source File

SOURCE=.\src\joy.h
# End Source File
# Begin Source File

SOURCE=.\src\menuscr.c
# End Source File
# Begin Source File

SOURCE=.\src\options.c
# End Source File
# Begin Source File

SOURCE=.\src\options.h
# End Source File
# Begin Source File

SOURCE=.\src\optscr.c
# End Source File
# Begin Source File

SOURCE=.\src\playlist.c
# End Source File
# Begin Source File

SOURCE=.\src\playlist.h
# End Source File
# Begin Source File

SOURCE=.\src\racescr.c
# End Source File
# Begin Source File

SOURCE=.\src\rbtree.c
# End Source File
# Begin Source File

SOURCE=.\src\rbtree.h
# End Source File
# Begin Source File

SOURCE=.\src\resman.c
# End Source File
# Begin Source File

SOURCE=.\src\resman.h
# End Source File
# Begin Source File

SOURCE=.\src\scene.c
# End Source File
# Begin Source File

SOURCE=.\src\scene.h
# End Source File
# Begin Source File

SOURCE=.\src\screens.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite.h
# End Source File
# Begin Source File

SOURCE=.\src\timer.h
# End Source File
# Begin Source File

SOURCE=.\src\track.c
# End Source File
# Begin Source File

SOURCE=.\src\track.h
# End Source File
# Begin Source File

SOURCE=.\src\treestor.c
# End Source File
# Begin Source File

SOURCE=.\src\treestor.h
# End Source File
# Begin Source File

SOURCE=.\src\ts_text.c
# End Source File
# Begin Source File

SOURCE=.\src\ui.c
# End Source File
# Begin Source File

SOURCE=.\src\ui.h
# End Source File
# Begin Source File

SOURCE=.\src\util.c
# End Source File
# Begin Source File

SOURCE=.\src\util.h
# End Source File
# End Group
# Begin Group "cgmath"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmath.h
# End Source File
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmmat.inl
# End Source File
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmmisc.inl
# End Source File
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmquat.inl
# End Source File
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmray.inl
# End Source File
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmvec3.inl
# End Source File
# Begin Source File

SOURCE=.\libs\cgmath\src\cgmvec4.inl
# End Source File
# End Group
# End Target
# End Project
