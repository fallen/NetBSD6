
/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/cpu.h>

void lm32_dtlb_invalidate_line(vaddr_t vaddr)
{
	vaddr |= 0x21;
	asm volatile ("wcsr tlbvaddr, %0" :: "r"(vaddr) : );
}

void lm32_itlb_invalidate_line(vaddr_t vaddr)
{
	vaddr |= 0x20;
	asm volatile ("wcsr tlbvaddr, %0" :: "r"(vaddr) : );
}
