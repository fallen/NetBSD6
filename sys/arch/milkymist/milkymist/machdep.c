/*	$NetBSD: machdep.c,v 1.43 2012/06/11 16:27:08 tsutsui Exp $	*/

/*-
 * Copyright (c) 1996, 1997, 1998, 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum and by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1982, 1987, 1990 The Regents of the University of California.
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
 *	@(#)machdep.c	7.4 (Berkeley) 6/3/91
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: machdep.c,v 1.43 2012/06/11 16:27:08 tsutsui Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/sysctl.h>
#include <sys/ksyms.h>
#include <sys/device.h>
#include <sys/module.h>
#include <sys/msgbuf.h>

#include <machine/intr.h>
#include <machine/pcb.h>
#include <machine/uart.h>
#include <lm32/cpuvar.h>

#include <dev/cons.h>

/* the following is used externally (sysctl_hw) */
char machine[] = MACHINE;
char machine_arch[] = MACHINE_ARCH;
char cpu_model[30];

/* Our exported CPU info; we can have only one. */
struct cpu_info cpu_info_store;
static struct pcb lwp0pcb;

void main(void) __attribute__((__noreturn__));
void milkymist_startup(void) __attribute__((__noreturn__));
void lm32_lwp0_init(void);

phys_ram_seg_t availmemr[1];

paddr_t phys_kernend;
paddr_t msgbuf_paddr;

struct cpu_softc cpu_softc[] = {
  [0] = {
    .cpu_ci = curcpu(),
  }
};

struct trapframe utf0;

void
milkymist_startup(void)
{
//	extern char edata[], end[];
	extern char _end[];
	/* Clear bss */
//	memset(edata, 0, end - edata);

	/* Initialize CPU ops. */
//	lm32_cpu_init();

	/* Console */
	consinit();

  printf("_end == %#lx\n", (long unsigned int)_end);
	phys_kernend = round_page((unsigned int)kern_virt_to_phy(_end));
  printf("phys_kernend == %#lx\n", phys_kernend);
  availmemr[0].start = IOM_RAM_BEGIN;
  availmemr[0].size = IOM_RAM_SIZE;
  
  msgbuf_paddr = (uintptr_t)(phys_kernend);
  phys_kernend += round_page(MSGBUFSIZE);
	printf(" msgbuf=%p", (void *)msgbuf_paddr);
  initmsgbuf((void *)msgbuf_paddr, round_page(MSGBUFSIZE));
	printf(" msgbuf=%p", (void *)msgbuf_paddr);

	uvm_setpagesize();
	/* Initialize proc0 u-area */
	lm32_lwp0_init();
  curcpu()->ci_curpm = pmap_kernel();
  curcpu()->ci_tlb_info = &pmap_tlb0_info;
  curcpu()->ci_softc = &cpu_softc[0];
  curcpu()->ci_cpl = IPL_HIGH;
  curcpu()->ci_idepth = -1;
  pmap_tlb_info_init(&pmap_tlb0_info);

  availmemr[0].start += (phys_kernend - IOM_RAM_BEGIN);
  availmemr[0].size -= (phys_kernend - IOM_RAM_BEGIN + 2*NBPG);
  /* 2*NBPG are reserved for kernel stack at the end of
   * physical memory */

	/* Initialize pmap and start to address translation */
	pmap_bootstrap(phys_kernend, availmemr);
	printf("IOM_RAM_SIZE == %d\n", (int)IOM_RAM_SIZE);
	printf("phys_kernend == %#lx\n", (long unsigned int)phys_kernend);


	/* Debugger. */
#if defined(KGDB) && (NSCIF > 0)
	if (scif_kgdb_init() == 0) {
		kgdb_debug_init = 1;
		kgdb_connect(1);
	}
#endif /* KGDB && NSCIF > 0 */

	/* Jump to main */
	__asm volatile(
		"b	%0"
		:: "r"(main));
	/* NOTREACHED */
	while (1)
		;
}

void lm32_lwp0_init(void)
{
	struct cpu_info *ci = curcpu();

	lwp0.l_cpu = ci;
	memset(&lwp0pcb, 0, sizeof(lwp0pcb));
	uvm_lwp_setuarea(&lwp0, (vaddr_t) &lwp0pcb);
  lwp0.l_md.md_utf = &utf0;
  ci->ci_curlwp = &lwp0;

}

void
consinit(void)
{
	static int initted = 0;

	if (initted)
		return;
	initted = 1;

	if (milkymist_uart_cnattach())
	{
		panic("Cannot init Milkymist serial console");
	}
}

void
cpu_startup(void)
{

	strcpy(cpu_model, "LatticeMico32\n");

//	lm32_startup();
}

SYSCTL_SETUP(sysctl_machdep_setup, "sysctl machdep subtree setup")
{

	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_NODE, "machdep", NULL,
		       NULL, 0, NULL, 0,
		       CTL_MACHDEP, CTL_EOL);

	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_STRUCT, "console_device", NULL,
		       sysctl_consdev, 0, NULL, sizeof(dev_t),
		       CTL_MACHDEP, CPU_CONSDEV, CTL_EOL);
}

void cpu_reset(void)
{
	asm volatile("rcsr r11, EBA\n\t"
		     "b r11" ::: "r11" );
}

void
cpu_reboot(int howto, char *bootstr)
{
#ifdef KLOADER
	struct kloader_bootinfo kbi;
#endif
	static int waittime = -1;

	if (cold) {
		howto |= RB_HALT;
		goto haltsys;
	}

#ifdef KLOADER
	/* No bootinfo is required. */
	kloader_bootinfo_set(&kbi, 0, NULL, NULL, true);
	if ((howto & RB_HALT) == 0) {
		if ((howto & RB_STRING) && bootstr != NULL) {
			printf("loading a new kernel: %s\n", bootstr);
			kloader_reboot_setup(bootstr);
		}
	}
#endif

	boothowto = howto;
	if ((howto & RB_NOSYNC) == 0 && waittime < 0) {
		waittime = 0;
		vfs_shutdown();
		/*
		 * If we've been adjusting the clock, the todr
		 * will be out of synch; adjust it now.
		 */
#if 0
		resettodr();
#endif
	}

	/* Disable interrupts. */
	splhigh();

	/* Do a dump if requested. */
//	if ((howto & (RB_DUMP | RB_HALT)) == RB_DUMP)
//		dumpsys();

 haltsys:
	doshutdownhooks();

	pmf_system_shutdown(boothowto);

	if (howto & RB_HALT) {
		printf("\n");
		printf("The operating system has halted.\n");
		printf("Please press any key to reboot.\n\n");
		cngetc();
	}

#ifdef KLOADER
	else if ((howto & RB_STRING) && bootstr != NULL) {
		kloader_reboot();
		printf("\nFailed to load a new kernel.\n");
		cngetc();
	}
#endif

	printf("rebooting...\n");
	cpu_reset();
	for(;;)
		;
	/*NOTREACHED*/
}

void lm32_cpu_idle(void)
{
	asm volatile("nop");
}

void
cpu_dumpconf(void)
{
}
