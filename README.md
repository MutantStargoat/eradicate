Eradicate
=========
Futuristic arcade racing game for DOS.
Written for the MS-DOS game jam: https://itch.io/jam/dos-game-jam-2

![screenshot](http://nuclear.mutantstargoat.com/sw/games/eradicate/shots/shot1-game-thumb.jpg)

Minimum requirements:
  - Pentium MMX 166MHz
  - 32MB RAM
  - SVGA video card supporting 16bit color
  - Any ISA sound card

Recommended hardware:
  - Pentium2 350MHz
  - 32MB RAM
  - SVGA video card supporting 16bit color, and VBE 2.0+
  - Any ISA sound card
  - Any gameport-compatible gamepad (TODO)

Eradicate attempts to auto-detect the installed sound card, and only asks the
user for manual sound card configuration if the auto-detection fails. If for
some reason your sound card is not detected correctly, or if you wish to change
the current settings, start the game with the `-setup` option. You will only
need to do this once; your choices are automatically saved, and will be used
every time you start the game in the future.

The cross-platform build uses OpenGL to display and scale the frame buffer,
either in a window, or fullscreen. To toggle between windowed and fullscreen
modes, simply press alt+enter, and the game will remember your choice next time
it starts.  Note that OpenGL is not used for 3D rendering; rendering is still
performed in software, and will be identical to the DOS build.


Controls
--------
  - Arrow keys or WASD: accelerate/break and turn
  - `tab`: skip to the next music track
  - `+`/`-`: change sound volume


Music
-----
Eradicate plays music tracks randomly from two directories:
  - `data/musgame` for in-game music.
  - `data/musmenu` for music to be played while in the game menus.

The release archive contains a selection of music tracks downloaded from
*modarchive*, but you can drop your own music in the jukebox directories.
Supported formats are:
  - Fast tracker extended modules (`.xm`).
  - Pro tracker 4-channel modules (`.mod`).
  - Scream tracker 3 modules (`.s3m`).
  - Impulse tracker modules (`.it`).

See https://modarchive.org for a wide selection of compatible music files.


Download
--------

Release v0.2 (jam bugfixes, includes source and binaries for DOS, GNU/Linux and Windows):
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/eradicate-0.2.tar.gz
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/eradicate-0.2.zip
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/erad02.zip

Release v0.1 (jam release, includes source and binaries for DOS, GNU/Linux and Windows):
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/eradicate-0.1.tar.gz
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/eradicate-0.1.zip
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/erad01.zip

The third link is a minimal archive with strictly 8.3 filenames for DOS versions
without FAT32 long filename support.

Source code repository:
  - https://github.com/MutantStargoat/eradicate

Release archives include the data files. If you got the source code from the github
repository, either copy the data directory from a release, or get the data from
the subversion data repository:
  - svn co svn://mutantstargoat.com/datadirs/eradicate data


License
-------
Copyright (C) 2020-2022 John Tsiombikas <nuclear@mutantstargoat.org>

This program is free software. Feel free to use, modify and/or redistribute it,
under the terms of the GNU General Public License version 3, or at your option
any later version published by the Free Software Foundation.
See COPYING for details.

Copyright for music files bundled with the releases belong to their respective
authors. This is not a commercial game, it's a free/open source project and
will never be sold for profit, but if you own the copyright to some bundled
track and would like it removed from future releases, please contact me.


Build DOS version
-----------------
First copy `libmidas.a` (DJGPP) and/or `midas.lib` (Watcom) from `data/bin` to
`libs/midas`.

  - *Watcom*: change into `libs/imago` and type `wmake`, then return to project
    root, and type `wmake`.
  - *DJGPP*: type `make -f Makefile.dj`.


Build cross-platform version
----------------------------
The cross-platform version is the best way to play eradicate on modern
computers. It uses OpenGL to present and scale the framebuffer and is very fast
and flexible on modern hardware.

To build the cross-platform version, under UNIX or windows with msys2/mingw32,
Just type `make`. Requires GNU make, some version of GCC, and OpenGL.

To build using visual studio, go into the vcbuild directory and execute
`setupvc2005.bat`. This will copy the visual studio 2005 project files to their
proper locations throughout the source tree. Change back into the project root
directory and open `eradicate.sln`. If you have a more recent version of visual
studio, it *should* be able to convert the project files automatically the first
time.

An experimental version of eradicate using OpenGL for rendering can be compiled
by changing the `renderer` build option at the top of `GNUmakefile` from
`software` to `opengl`. This version performs significantly better than the
software rendering one, on anything with reasonable 3D rendering hardware and
a slow processor, like an SGI O2, or a Pentium1 with a TNT2 graphics card.


Build retro windows version (DirectDraw)
----------------------------------------
The DirectDraw version is mainly intended for old computers with 2D graphics
cards, which are unable to run the cross-platform version with reasonable
performance.

Currently the DirectDraw version can only be built using the MSVC 6.0 project
files. Enter `vcbuild` and execute `setupvc6.bat`. This will copy the vc6
project files to the appropriate places throughout the source tree. Then return
to the project root directory and open `eradicate.dsw`. If you have a more
recent version of visual studio, it *should* be able to convert the project
files automatically the first time.
