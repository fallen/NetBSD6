/*	$NetBSD: cpu.h,v 1.3 2002/03/04 14:36:13 uch Exp $	*/

#define curcpu() (&cpu_info_store)
extern struct cpu_info cpu_info_store;

#include <lm32/cpu.h>
