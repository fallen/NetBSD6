/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lib/libkern/libkern.h>
#include <machine/psl.h>

struct irqhandler {
	unsigned int mask;
	int (*irq_handler)(void *);
};

struct irqhandler irqhandlers[32];

void init_irqhandlers_array(void)
{
	memset(irqhandlers, 0, sizeof(irqhandlers));
}

unsigned int lm32_dispatch_irq(unsigned int irq_pending_mask, void *arg)
{
	unsigned int i;
	int s;

	for (i = 0 ; i < 32 ; i++)
	{
		if ((irq_pending_mask & irqhandlers[i].mask)
		    && (irqhandlers[i].irq_handler != NULL))
		{
			s = spl0();
			if (irqhandlers[i].irq_handler(arg))
			{
				splx(s);
				return 1;
			}
			splx(s);
		}
	}

	printf("spurious IRQ or no handler registered for this IRQ: IP == 0x%08X\n", irq_pending_mask);
	return 0;
}

void lm32_intrhandler_register(int irqmask, int (*func)(void *))
{
	unsigned int i;
	for (i = 0 ; i < 32 ; i++)
	{
		if (irqhandlers[i].irq_handler == NULL)
		{
			irqhandlers[i].mask = irqmask;
			irqhandlers[i].irq_handler = func;
			return;
		}
	}

	panic("No more slot to register irq handler\n");
}
