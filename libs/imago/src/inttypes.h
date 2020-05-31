/*
colcycle - color cycling image viewer
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef INT_TYPES_H_
#define INT_TYPES_H_

#if defined(__DOS__) || defined(__MSDOS__)
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef unsigned long intptr_t;
#else

#ifdef _MSC_VER
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;

#ifdef _WIN64
typedef __int64 intptr_t;
#else
typedef __int32 intptr_t;
#endif
#else	/* not msvc */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199900
#include <stdint.h>
#else
#include <sys/types.h>
#endif

#endif	/* end !msvc */
#endif	/* end !dos */

#endif	/* INT_TYPES_H_ */
