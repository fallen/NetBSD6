/*	$NetBSD: $	*/

#ifndef _MILKYMIST_PSL_H_
#define _MILKYMIST_PSL_H_

#include <lm32/psl.h>
#include <milkymist/intr.h>

static __inline void splx(int s)
{
	_set_irq_mask(s);
}

static __inline int _spl(int s)
{
	int level = _get_irq_mask();
	_set_irq_mask(s);
	return level;
}

#define spl0() 			_spl(ALL_IRQ)
#define splvm() 		_spl(NO_IRQ)
#define splsoftclock()		_spl(NO_IRQ)
#define splsoftnet()		_spl(NO_IRQ)
#define splsoftserial()		_spl(NO_IRQ)
#define splsoftbio()		_spl(NO_IRQ)
#define splsched() 		_spl(NO_IRQ)
#define splhigh() 		_spl(NO_IRQ)

#endif
