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
#define		IPL_VM		2
#define		IPL_SCHED	3
#define		IPL_HIGH	4
#define		NIPL		5

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

static __inline void _ack_irq(unsigned int irq)
{
	unsigned int __ip;
	asm volatile("rcsr %0, IP\n\t"
		     "or %0, %0, %1\n\t"
		     "wcsr IP, %0" : "=&r"(__ip) : "r"(irq) : );
}

void lm32_intrhandler_register(int irqmask, int (*func)(void *));
void init_irqhandlers_array(void);
unsigned int lm32_dispatch_irq(unsigned int irq_pending_mask, void *arg);

#endif
#endif
