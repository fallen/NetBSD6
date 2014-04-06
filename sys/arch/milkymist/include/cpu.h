/*	$NetBSD: $	*/

#ifndef _MILKYMIST_CPU_H_
#define _MILKYMIST_CPU_H_
#define curcpu() (&cpu_info_store)
extern struct cpu_info cpu_info_store;

#include <lm32/cpu.h>

#define CPU_CONSDEV 1

#endif
