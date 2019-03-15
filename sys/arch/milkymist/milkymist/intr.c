/*	$NetBSD: $	*/

/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <lm32/cpu.h>
#include <lib/libkern/libkern.h>

int IPL2IM[NIPL] = {
	/*UART_IRQ |*/ TIMER0_IRQ | TIMER1_IRQ, /* IPL_NONE */
	NO_IRQ, /* IPL_SOFTCLOCK | IPL_SOFTBIO | IPL_SOFTNET | IPL_SOFTSERIAL */
	NO_IRQ,   /* IPL_VM */
	NO_IRQ,   /* IPL_SCHED */
	NO_IRQ,  /* IPL_HIGH */
};

int _splraise(int ipl)
{
	struct cpu_info * const ci = curcpu();
	const int old_ipl = ci->ci_cpl;

	if (ci->ci_cpl == ipl)
		return ipl;

	KASSERT(ipl < NIPL);

	if (old_ipl < ipl)
		splx(ipl);

	return old_ipl;
}

int _spllower(int ipl)
{
	struct cpu_info * const ci = curcpu();
	const int old_ipl = ci->ci_cpl;

	if (ci->ci_cpl == ipl)
		return ipl;

	KASSERT(ipl < NIPL);

	if (old_ipl > ipl)
		splx(ipl);

	return old_ipl;
}

void __isr(unsigned int irq_pending_mask, unsigned int return_address, struct trapframe *tf)
{
	struct trapframe *vtf = (struct trapframe *)kern_phy_to_virt_ramwindow(tf); // virtual pointer to trapframe

	lm32_dispatch_irq(irq_pending_mask, vtf);

	return;
}
