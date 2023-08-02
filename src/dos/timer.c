/* for sound we use MIDAS, which takes over the PIT and we can't use it
 * therefore only compile this file for NO_SOUND builds.
 */
#ifdef NO_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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

#include "pit8254.h"
#include "inttypes.h"
#include "util.h"
#include "dosutil.h"

#define PIT_TIMER_INTR	8
#define DOS_TIMER_INTR	0x1c

/* macro to divide and round to the nearest integer */
#define DIV_ROUND(a, b) \
	((a) / (b) + ((a) % (b)) / ((b) / 2))

static void set_timer_reload(int reload_val);
static void cleanup(void);

#ifdef __WATCOMC__
#define INTERRUPT	__interrupt __far

static void INTERRUPT dos_timer_intr();

static void (INTERRUPT *prev_timer_intr)();
#endif

#ifdef __DJGPP__
#define INTERRUPT

static _go32_dpmi_seginfo intr, prev_intr;
#endif

static void INTERRUPT timer_irq();

static volatile unsigned long ticks;
static unsigned long tick_interval, ticks_per_dos_intr;
static int inum;

void init_timer(int res_hz)
{
	_disable();

	if(res_hz > 0) {
		int reload_val = DIV_ROUND(OSC_FREQ_HZ, res_hz);
		set_timer_reload(reload_val);

		tick_interval = DIV_ROUND(1000, res_hz);
		ticks_per_dos_intr = DIV_ROUND(65535L, reload_val);

		inum = PIT_TIMER_INTR;
#ifdef __WATCOMC__
		prev_timer_intr = _dos_getvect(inum);
		_dos_setvect(inum, timer_irq);
#endif
#ifdef __DJGPP__
		_go32_dpmi_get_protected_mode_interrupt_vector(inum, &prev_intr);
		intr.pm_offset = (intptr_t)timer_irq;
		intr.pm_selector = _go32_my_cs();
		_go32_dpmi_allocate_iret_wrapper(&intr);
		_go32_dpmi_set_protected_mode_interrupt_vector(inum, &intr);
#endif
	} else {
		tick_interval = 55;

		inum = DOS_TIMER_INTR;
#ifdef __WATCOMC__
		prev_timer_intr = _dos_getvect(inum);
		_dos_setvect(inum, dos_timer_intr);
#endif
#ifdef __DJGPP__
		assert(0);
#endif
	}
	_enable();

	atexit(cleanup);
}

static void cleanup(void)
{
	if(!inum) {
		return; /* init hasn't ran, there's nothing to cleanup */
	}

	_disable();
	if(inum == PIT_TIMER_INTR) {
		/* restore the original timer frequency */
		set_timer_reload(65535);
	}

	/* restore the original interrupt handler */
#ifdef __WATCOMC__
	_dos_setvect(inum, prev_timer_intr);
#endif
#ifdef __DJGPP__
	_go32_dpmi_set_protected_mode_interrupt_vector(inum, &prev_intr);
	_go32_dpmi_free_iret_wrapper(&intr);
#endif

	_enable();
}

void reset_timer(void)
{
	ticks = 0;
}

unsigned long get_msec(void)
{
	return ticks * tick_interval;
}

void sleep_msec(unsigned long msec)
{
	unsigned long wakeup_time = ticks + msec / tick_interval;
	while(ticks < wakeup_time) {
#ifdef USE_HLT
		halt();
#endif
	}
}

static void set_timer_reload(int reload_val)
{
	outp(PORT_CMD, CMD_CHAN0 | CMD_ACCESS_BOTH | CMD_OP_SQWAVE);
	outp(PORT_DATA0, reload_val & 0xff);
	outp(PORT_DATA0, (reload_val >> 8) & 0xff);
}

#ifdef __WATCOMC__
static void INTERRUPT dos_timer_intr()
{
	ticks++;
	_chain_intr(prev_timer_intr);	/* DOES NOT RETURN */
}
#endif

/* first PIC command port */
#define PIC1_CMD	0x20
/* end of interrupt control word */
#define OCW2_EOI	(1 << 5)

static void INTERRUPT timer_irq()
{
	static unsigned long dos_ticks;

	ticks++;

#ifdef __WATCOMC__
	if(++dos_ticks >= ticks_per_dos_intr) {
		/* I suppose the dos irq handler does the EOI so I shouldn't
		 * do it if I am to call the previous function
		 */
		dos_ticks = 0;
		_chain_intr(prev_timer_intr);	/* XXX DOES NOT RETURN */
		return;	/* just for clarity */
	}
#endif

	/* send EOI to the PIC */
	outp(PIC1_CMD, OCW2_EOI);
}

#endif	/* NO_SOUND */
