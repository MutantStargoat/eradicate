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
#define KEYB_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>

#ifdef __WATCOMC__
#include <i86.h>
#endif
#ifdef __DJGPP__
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#endif

#include "keyb.h"
#include "scancode.h"
#include "inttypes.h"
#include "dosutil.h"

#define KB_INTR		0x9
#define KB_PORT		0x60

#define PIC1_CMD_PORT	0x20
#define OCW2_EOI		(1 << 5)

#ifdef __WATCOMC__
#define INTERRUPT __interrupt __far

#define DONE_INIT	(prev_handler)
static void (INTERRUPT *prev_handler)();
#endif

#ifdef __DJGPP__
#define INTERRUPT

#define DONE_INIT prev_intr.pm_offset
static _go32_dpmi_seginfo intr, prev_intr;
#endif

static void INTERRUPT kbintr();

static int *buffer;
static int buffer_size, buf_ridx, buf_widx;
static int last_key;

static unsigned int num_pressed;
static unsigned char keystate[256];

#define ADVANCE(x)	((x) = ((x) + 1) % buffer_size)

int kb_init(int bufsz)
{
	if(DONE_INIT) {
		fprintf(stderr, "keyboard driver already initialized!\n");
		return 0;
	}

	buffer_size = bufsz;
	if(buffer_size && !(buffer = malloc(buffer_size * sizeof *buffer))) {
		fprintf(stderr, "failed to allocate input buffer, continuing without\n");
		buffer_size = 0;
	}
	buf_ridx = buf_widx = 0;
	last_key = -1;

	memset(keystate, 0, sizeof keystate);
	num_pressed = 0;

	/* set our interrupt handler */
	_disable();
#ifdef __WATCOMC__
	prev_handler = _dos_getvect(KB_INTR);
	_dos_setvect(KB_INTR, kbintr);
#endif
#ifdef __DJGPP__
	_go32_dpmi_get_protected_mode_interrupt_vector(KB_INTR, &prev_intr);
	intr.pm_offset = (intptr_t)kbintr;
	intr.pm_selector = _go32_my_cs();
	_go32_dpmi_allocate_iret_wrapper(&intr);
	_go32_dpmi_set_protected_mode_interrupt_vector(KB_INTR, &intr);
#endif
	_enable();

	return 0;
}

void kb_shutdown(void)
{
	if(!DONE_INIT) {
		return;
	}

	/* restore the original interrupt handler */
	_disable();
#ifdef __WATCOMC__
	_dos_setvect(KB_INTR, prev_handler);
#endif
#ifdef __DJGPP__
	_go32_dpmi_set_protected_mode_interrupt_vector(KB_INTR, &prev_intr);
	_go32_dpmi_free_iret_wrapper(&intr);
#endif
	_enable();

	free(buffer);
}

int kb_isdown(int key)
{
	switch(key) {
	case KB_ANY:
		return num_pressed;

	case KB_ALT:
		return keystate[KB_LALT] + keystate[KB_RALT];

	case KB_CTRL:
		return keystate[KB_LCTRL] + keystate[KB_RCTRL];
	}

	if(isalpha(key)) {
		key = tolower(key);
	}
	return keystate[key];
}

#ifdef __WATCOMC__
void halt(void);
#pragma aux halt = \
	"sti" \
	"hlt";
#endif

#ifdef __DJGPP__
#define halt() asm volatile("sti\n\thlt\n\t")
#endif

void kb_wait(void)
{
	int key;
	while((key = kb_getkey()) == -1) {
#ifdef USE_HLT
		/* put the processor to sleep while waiting for keypresses, but first
		 * make sure interrupts are enabled, or we'll sleep forever
		 */
		halt();
#endif
	}
	kb_putback(key);
}

int kb_getkey(void)
{
	int res;

	if(buffer) {
		if(buf_ridx == buf_widx) {
			return -1;
		}
		res = buffer[buf_ridx];
		ADVANCE(buf_ridx);
	} else {
		res = last_key;
		last_key = -1;
	}
	return res;
}

void kb_putback(int key)
{
	if(buffer) {
		/* go back a place */
		if(--buf_ridx < 0) {
			buf_ridx += buffer_size;
		}

		/* if the write end hasn't caught up with us, go back one place
		 * and put it there, otherwise just overwrite the oldest key which
		 * is right where we were.
		 */
		if(buf_ridx == buf_widx) {
			ADVANCE(buf_ridx);
		}

		buffer[buf_ridx] = key;
	} else {
		last_key = key;
	}
}

static void INTERRUPT kbintr()
{
	unsigned char code;
	int key, c, press;
	static int ext;

	code = inp(KB_PORT);

	if(code == 0xe0) {
		ext = 1;
		goto eoi;
	}

	if(code & 0x80) {
		press = 0;
		code &= 0x7f;

		if(num_pressed > 0) {
			num_pressed--;
		}
	} else {
		press = 1;

		num_pressed++;
	}

	if(ext) {
		key = scantbl_ext[code];
		c = key;
		ext = 0;
	} else {
		key = scantbl[code];
		c = (keystate[KB_LSHIFT] | keystate[KB_RSHIFT]) ? scantbl_shift[code] : key;
	}

	if(press) {
		/* append to buffer */
		last_key = c;
		if(buffer_size > 0) {
			buffer[buf_widx] = c;
			ADVANCE(buf_widx);
			/* if the write end overtook the read end, advance the read end
			 * too, to discard the oldest keypress from the buffer
			 */
			if(buf_widx == buf_ridx) {
				ADVANCE(buf_ridx);
			}
		}
	}

	/* and update keystate table */
	keystate[key] = press;

eoi:
	outp(PIC1_CMD_PORT, OCW2_EOI);	/* send end-of-interrupt */
}
