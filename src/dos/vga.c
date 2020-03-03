#include "vga.h"
#include "cdpmi.h"

int vga_setmode(int mode)
{
	struct dpmi_regs regs = {0};

	regs.eax = mode;	/* func 00 | mode */
	dpmi_int(0x10, &regs);
	return 0;
}
