Eradicate
=========
Futuristic arcade racing game for DOS.
Written for the MS-DOS game jam: https://itch.io/jam/dos-game-jam-2

No releases yet, as it's not finished. Check back later, or grab the source and
try your luck if you want to follow along.

Grab the data with:
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

License
-------
Copyright (C) 2020 John Tsiombikas <nuclear@mutantstargoat.org>

This program is free software. Feel free to use, modify and/or redistribute it,
under the terms of the GNU General Public License version 3, or at your option
any later version published by the Free Software Foundation.
See COPYING for details.

Build
-----
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
