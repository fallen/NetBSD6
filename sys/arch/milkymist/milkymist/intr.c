/*	$NetBSD: $	*/

/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <lm32/cpu.h>
#include <lib/libkern/libkern.h>

ipl_t _splraise(ipl_t level)
{
	struct cpu_info *ci = curcpu();
	ipl_t olevel;
	int psw;

	if (ci->ci_current_ipl == level)
		return level;
	olevel = ci->ci_current_ipl;

//	KASSERT(level < NIPL);

	ci->ci_current_ipl = max(level, olevel);

	asm volatile("rcsr %0, PSW" : "=r"(psw) :: );

	if (level == IPL_NONE)
		psw |= LM32_CSR_PSW_IE;
	else
		psw &= ~(LM32_CSR_PSW_IE);

	asm volatile("wcsr PSW, %0" :: "r"(psw) : );

	return olevel;
}

void __isr(unsigned int irq_pending_mask, unsigned int return_address, struct trapframe *tf)
{
 unsigned int psw;
	struct trapframe *vtf = (struct trapframe *)kern_phy_to_virt_ramwindow(tf); // virtual pointer to trapframe

	lm32_dispatch_irq(irq_pending_mask, vtf);

  /* return to _real_interrupt_handler with mmu OFF */
  psw = PSW_DTLBE | PSW_ITLBE;
  asm volatile("wcsr PSW, %0" :: "r"(psw) : );
  asm volatile(
              "mv ea, %0\n\t"
              "eret"
              :: "r"(return_address)
              : );

}
