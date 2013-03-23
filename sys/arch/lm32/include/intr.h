#ifndef _LM32_INTR_H_
#define _LM32_INTR_H

#if defined(_KERNEL)

#include <lm32/psl.h>

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
