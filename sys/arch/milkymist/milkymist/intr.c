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
