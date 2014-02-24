/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/cpu.h>
#include <lib/libkern/libkern.h>

ipl_t _splraise(ipl_t level)
{
	struct cpu_info *ci = curcpu();
	ipl_t olevel;
	int psw;
	int ie;

	if (ci->ci_current_ipl == level)
		return level;
	olevel = ci->ci_current_ipl;

//	KASSERT(level < NIPL);

	ci->ci_current_ipl = max(level, olevel);

	asm volatile("rcsr %0, PSW" : "=r"(psw) :: );
	asm volatile("rcsr %0, IE" : "=r"(ie) :: );

	if (level == IPL_NONE)
	{
		psw |= LM32_CSR_PSW_IE;
		ie |= 1;
	} else {
		psw &= ~(LM32_CSR_PSW_IE);
		ie &= ~1;
	}

	asm volatile("wcsr PSW, %0" :: "r"(psw) : );
	// FIXME: This will not be needed anymore when Qemu will shadow IE to PSW.
	asm volatile("wcsr IE, %0" :: "r"(ie) : );

	return olevel;
}

void __isr(unsigned int irq_pending_mask, unsigned int return_address)
{
 unsigned int psw;


//TODO: make sure we pass trapframe as argument to irq and lm32_dispatch_irq
	lm32_dispatch_irq(irq_pending_mask, NULL);

  /* return to _real_interrupt_handler with mmu OFF */
  psw = PSW_DTLBE | PSW_ITLBE;
  asm volatile("wcsr PSW, %0" :: "r"(psw) : );
  asm volatile(
              "mv ea, %0\n\t"
              "eret"
              :: "r"(return_address)
              : );

}
