Eradicate
=========
Futuristic arcade racing game for DOS.
Written for the MS-DOS game jam: https://itch.io/jam/dos-game-jam-2

![screenshot](http://nuclear.mutantstargoat.com/sw/games/eradicate/shots/shot1-game-thumb.jpg)

Release v0.1 (jam release, includes source and binaries for DOS, GNU/Linux and Windows):
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/eradicate-0.1.tar.gz
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/eradicate-0.1.zip
  - http://nuclear.mutantstargoat.com/sw/games/eradicate/releases/erad01.zip

The third link is a minimal archive with only 8.3 filenames for DOS versions
without FAT32 long filename support.

Source code repository:
  - https://github.com/MutantStargoat/eradicate

Release archives include the data files. If you got the code from the github
repository, either copy the data directory from a release, or get the data from
the subversion data repository:
    svn co svn://mutantstargoat.com/datadirs/eradicate data

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
  - Any gameport-compatible gamepad

Eradicate attempts to auto-detect the installed sound card, and only asks the
user for manual sound card configuration if the auto-detection fails. If for
some reason your sound card is not detected correctly, or if you wish to change
the current settings, start the game with the `-setup` option. You will only
need to do this once; your choices are automatically saved, and will be used
every time you start the game in the future.

The GNU/Linux and Windows versions support framebuffer scaling. Use the FBSCALE
environment variable to set the scaling factor. For example: `export FBSCALE=2`
will result in 2x scaling. Also you can toggle between fullscreen and windowed
modes with alt-enter.

Controls
--------
  - Arrow keys or WASD: accelerate/break and turn
  - `tab`: skip to the next music track
  - `+`/`-`: change sound volume

License
-------
Copyright (C) 2020 John Tsiombikas <nuclear@mutantstargoat.org>

This program is free software. Feel free to use, modify and/or redistribute it,
under the terms of the GNU General Public License version 3, or at your option
any later version published by the Free Software Foundation.
See COPYING for details.

Build
-----
First copy `libmidas.a` (DJGPP) and/or `midas.lib` (Watcom) from `data/bin` to
`libs/midas`.

  - *Watcom*: change into `libs/imago` and type `wmake`, then return to project
    root, and type `wmake`.
  - *DJGPP*: type `make -f Makefile.dj`.

Build SDL version (x86 only)
----------------------------
To build the SDL version, you need:
  - libSDL 1.2 (32bit version) installed.
  - a x86 32bit compiler toolchain (only GCC tested).
  - GNU make.

Just type `make`.
