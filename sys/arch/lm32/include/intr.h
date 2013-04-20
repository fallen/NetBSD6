#ifndef _LM32_INTR_H_
#define _LM32_INTR_H

#if defined(_KERNEL)

#include <lm32/psl.h>

/* Define the various Interrupt Priority Levels */

/* Hardware Interrupt Priority Levels are not mutually exclusive. */

#define		IPL_NONE	0
#define		IPL_SOFTCLOCK	1
#define		IPL_SOFTBIO	1
#define		IPL_SOFTNET	1
#define		IPL_SOFTSERIAL	1
#define		IPL_VM		1
#define		IPL_SCHED	1
#define		IPL_HIGH	1
#define		NIPL		2

static __inline int _get_irq_mask(void)
{
	int mask;

	__asm volatile ("rcsr %0, IM" : "=r"(mask) :: );

	return mask;
}

static __inline void _set_irq_mask(int mask)
{
	__asm volatile ("wcsr IM, %0" :: "r"(mask) : );
}

#endif
#endif
