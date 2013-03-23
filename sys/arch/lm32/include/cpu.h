/*	$NetBSD: cpu.h,v 1.53 2012/10/27 17:18:13 chs Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)cpu.h	5.4 (Berkeley) 5/9/91
 */

#ifndef _LM32_CPU_H_
#define _LM32_CPU_H_

#if defined(_KERNEL) || defined(_STANDALONE)
#include <sys/types.h>
#else
#include <stdbool.h>
#endif /* _KERNEL || _STANDALONE */

#if defined(_KERNEL) || defined(_KMEMUSER)

/*
 * Definitions unique to lm32 cpu support.
 */
#include <machine/frame.h>
#include <machine/pte.h>

#include <sys/cpu_data.h>
#include <sys/evcnt.h>
#include <sys/device_if.h> /* for device_t */


struct intrsource;
struct pmap;

/*
 * a bunch of this belongs in cpuvar.h; move it later..
 */

struct cpu_info {
	struct cpu_data ci_data;	/* MI per-cpu data */
	cpuid_t	ci_cpuid;
	device_t ci_dev;		/* pointer to our device */
	int ci_current_ipl;
	struct cpu_info *ci_self;	/* self-pointer */
	void	*ci_tlog_base;		/* Trap log base */
	int32_t ci_tlog_offset;		/* Trap log current offset */

	/*
	 * Private members.
	 */
	struct evcnt ci_tlb_evcnt;	/* tlb shootdown counter */
	struct pmap *ci_pmap;		/* current pmap */
	int ci_want_pmapload;		/* pmap_load() is needed */
	int ci_curldt;		/* current LDT descriptor */
	int ci_nintrhand;	/* number of H/W interrupt handlers */
	uint64_t ci_scratch;
	uintptr_t ci_pmap_data[128 / sizeof(uintptr_t)];

	volatile int	ci_mtx_count;	/* Negative count of spin mutexes */
	volatile int	ci_mtx_oldspl;	/* Old SPL at this ci_idepth */

	uint32_t ci_flags;		/* flags; see below */

	uint32_t	ci_signature;	 /* LM32 cpuid type */
	uint32_t	ci_vendor[4];	 /* vendor string */
	uint32_t	ci_cpu_serial[3]; /* PIII serial number */

	const struct cpu_functions *ci_func;  /* start/stop functions */
	struct trapframe *ci_ddb_regs;

	u_int ci_cflush_lsize;	/* CFLUSH insn line size */
	char *ci_doubleflt_stack;
	char *ci_ddbipi_stack;
	volatile int	ci_want_resched;
};


#endif /* _KERNEL || __KMEMUSER */

#ifdef _KERNEL
/*
 * We statically allocate the CPU info for the primary CPU (or,
 * the only CPU on uniprocessors), and the primary CPU is the
 * first CPU on the CPU info list.
 */
extern struct cpu_info cpu_info_primary;
extern struct cpu_info *cpu_info_list;

extern struct cpu_info cpu_info_store;
#define	curcpu()			(&cpu_info_store)

/*
 * definitions of cpu-dependent requirements
 * referenced in generic code
 */
#define	cpu_number()			0

#define	cpu_proc_fork(p1, p2)		/* nothing */

#define CPU_STARTUP(_ci, _target)	((_ci)->ci_func->start(_ci, _target))
#define CPU_STOP(_ci)	        	((_ci)->ci_func->stop(_ci))
#define CPU_START_CLEANUP(_ci)		((_ci)->ci_func->cleanup(_ci))

#define aston(l, why)		((l)->l_md.md_astpending |= (why))

void cpu_boot_secondary_processors(void);
void cpu_init_idle_lwps(void);
void cpu_init_msrs(struct cpu_info *, bool);
void cpu_load_pmap(struct pmap *, struct pmap *);
void cpu_broadcast_halt(void);
void cpu_kick(struct cpu_info *);

#define	curpcb			((struct pcb *)lwp_getpcb(curlwp))

/*
 * Arguments to hardclock, softclock and statclock
 * encapsulate the previous machine state in an opaque
 * clockframe; for now, use generic intrframe.
 */
struct clockframe {
	struct registers cf_if;
	int tf_pc;
};

#define CLKF_USERMODE(framep) (0)
#define CLKF_PC(framep) ((framep)->tf_pc)
#define CLKF_INTR(framep) (0)

/*
 * Give a profiling tick to the current process when the user profiling
 * buffer pages are invalid.  On the i386, request an ast to send us
 * through trap(), marking the proc as needing a profiling tick.
 */
extern void	cpu_need_proftick(struct lwp *l);

/*
 * Notify the LWP l that it has a signal pending, process as soon as
 * possible.
 */
extern void	cpu_signotify(struct lwp *);

/*
 * We need a machine-independent name for this.
 */
extern void (*delay_func)(unsigned int);
struct timeval;

#define	DELAY(x)		(*delay_func)(x)
#define delay(x)		(*delay_func)(x)

extern int cputype;
extern int cpuid_level;
extern int cpu_class;

extern void (*lm32_cpu_idle)(void);
#define	cpu_idle() (*lm32_cpu_idle)()

/* machdep.c */
void	dumpconf(void);
void	cpu_reset(void);

/* identcpu.c */
void 	cpu_probe(struct cpu_info *);
void	cpu_identify(struct cpu_info *);

/* locore.s */
struct region_descriptor;
void	lgdt(struct region_descriptor *);

struct pcb;
void	savectx(struct pcb *);
void	lwp_trampoline(void);

/* cpu.c */

void	cpu_probe_features(struct cpu_info *);

/* npx.c */
void	npxsave_lwp(struct lwp *, bool);
void	npxsave_cpu(bool);

/* vm_machdep.c */
paddr_t	kvtop(void *);

#endif /* _KERNEL */

#if defined(_KERNEL) || defined(_KMEMUSER)
#include <machine/psl.h>	/* Must be after struct cpu_info declaration */
#endif /* _KERNEL || __KMEMUSER */

/*
 * Structure for CPU_DISKINFO sysctl call.
 * XXX this should be somewhere else.
 */
#define MAX_BIOSDISKS	16

struct disklist {
	int dl_nbiosdisks;			   /* number of bios disks */
	struct biosdisk_info {
		int bi_dev;			   /* BIOS device # (0x80 ..) */
		int bi_cyl;			   /* cylinders on disk */
		int bi_head;			   /* heads per track */
		int bi_sec;			   /* sectors per track */
		uint64_t bi_lbasecs;		   /* total sec. (iff ext13) */
#define BIFLAG_INVALID		0x01
#define BIFLAG_EXTINT13		0x02
		int bi_flags;
	} dl_biosdisks[MAX_BIOSDISKS];

	int dl_nnativedisks;			   /* number of native disks */
	struct nativedisk_info {
		char ni_devname[16];		   /* native device name */
		int ni_nmatches; 		   /* # of matches w/ BIOS */
		int ni_biosmatches[MAX_BIOSDISKS]; /* indices in dl_biosdisks */
	} dl_nativedisks[1];			   /* actually longer */
};
#endif /* !_LM32_CPU_H_ */
