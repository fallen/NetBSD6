/*	$NetBSD: $	*/

#ifndef _MILKYMIST_PSL_H_
#define _MILKYMIST_PSL_H_

#ifdef _KERNEL
#include <lm32/psl.h>
#include <milkymist/intr.h>

extern int IPL2IM[NIPL];
int _splraise(int ipl);
int _spllower(int ipl);

static __inline void splx(int ipl)
{
	struct cpu_info * const ci = curcpu();
	const int old_ipl = ci->ci_cpl;

	if (ipl == old_ipl)
		return;

	_set_irq_mask(IPL2IM[ipl]);
}

#define spl0() 			_spllower(IPL_NONE)
#define splvm() 		_splraise(IPL_VM)
#define splsoftclock()		_splraise(IPL_SOFTCLOCK)
#define splsoftnet()		_splraise(IPL_SOFTNET)
#define splsoftserial()		_splraise(IPL_SOFTSERIAL)
#define splsoftbio()		_splraise(NO_IRQ)
#define splsched() 		_splraise(IPL_SCHED)
#define splhigh() 		_splraise(IPL_HIGH)

#endif /* _KERNEL */
#endif
