/*
DOS interrupt-based keyboard driver.
Copyright (C) 2013  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License  for more details.

You should have received a copy of the GNU General Public License
along with the program. If not, see <http://www.gnu.org/licenses/>
*/
#ifndef KEYB_H_
#define KEYB_H_

#include "game.h"

#define KB_ANY		(-1)
#define KB_ALT		(-2)
#define KB_CTRL		(-3)
#define KB_SHIFT	(-4)

#ifdef __cplusplus
extern "C" {
#endif

int kb_init(int bufsz);	/* bufsz can be 0 for no buffered keys */
void kb_shutdown(void); /* don't forget to call this at the end! */

/* Boolean predicate for testing the current state of a particular key.
 * You may also pass KB_ANY to test if any key is held down.
 */
int kb_isdown(int key);

/* waits for any keypress */
void kb_wait(void);

/* removes and returns a single key from the input buffer.
 * If buffering is disabled (initialized with kb_init(0)), then it always
 * returns the last key pressed.
 */
int kb_getkey(void);

void kb_putback(int key);

#ifdef __cplusplus
}
#endif

#endif	/* KEYB_H_ */
