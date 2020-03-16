gph-cmath: C math library for graphics
======================================

About
-----
gph-cmath is a C math library for graphics programs. It provides a plethora of
operations on vectors, matrices and quaternions, among other things.

It's conceptually a companion to my C++ math library
[gph-math](http://github.com/jtsiomb/gph-math), but where gph-math is designed
with intuitiveness and ease of use as the main priority, this C version is
designed to be as low-overhead as possible, making it more suitable for more
resource-constrained target systems.
For instance most functions modify their first argument, instead of doing an
extra copy to provide a more natural 3 operand interface. Leaving the copy to
the user for the cases where it's necessary.

License
-------
Copyright (C) 2016 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the MIT/X11 license. See LICENSE for details.

How to use
----------
There's nothing to build. All functions are static inline, defined in the header
files. Either type `make install` to install them system-wide, or just copy all
files under `src/` to your project source tree.
