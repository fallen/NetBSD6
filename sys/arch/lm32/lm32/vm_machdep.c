/*
 * Copyright (c) 1999-2004 Michael Shalayeff
 * Copyright (c) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/ptrace.h>
#include <sys/exec.h>
#include <sys/core.h>
#include <sys/pool.h>
#include <sys/cpu.h>
#include <sys/kcore.h>
#include <sys/kauth.h>
#include <sys/ras.h>
#include <dev/mm.h>

#include <machine/pmap.h>
#include <machine/pcb.h>
#include <machine/userret.h>

#include <uvm/uvm.h>

// TODO adapt the following function for lm32 pmap subsystem
static inline void
cpu_activate_pcb(struct lwp *l)
{
}


// TODO adapt the following function for lm32 pmap subsystem
void
cpu_lwp_fork(struct lwp *l1, struct lwp *l2, void *stack, size_t stacksize,
    void (*func)(void *), void *arg)
{
	/*
	 * If l1 != curlwp && l1 == &lwp0, we're creating a kernel thread.
	 */
	KASSERT(l1 == curlwp || l1 == &lwp0);

	struct pcb * const pcb1 = lwp_getpcb(l1);
	struct pcb * const pcb2 = lwp_getpcb(l2);

  (void)(pcb1);

  memset(pcb2, 0, sizeof(*pcb2));

  *pcb2 = *pcb1;

	const vaddr_t uv = uvm_lwp_getuarea(l2);
	struct trapframe * const tf = (struct trapframe *)(uv + USPACE) - 1;
	l2->l_md.md_utf = tf;
	*tf = *l1->l_md.md_utf;

	pcb2->pcb_onfault = NULL;
  pcb2->pcb_regs[_REG_SP] = (register_t)tf;
  pcb2->pcb_regs[_REG_R1] = (register_t)arg;
  pcb2->pcb_regs[_REG_R2] = (register_t)func;
  pcb2->pcb_regs[_REG_RA] = (register_t)lwp_trampoline;
}

/*
 * Map an IO request into kernel virtual address space.
 */
// TODO implement vmapbuf
int
vmapbuf(struct buf *bp, vsize_t len)
{
	return 0;
}

/*
 * Unmap IO request from the kernel virtual address space.
 */
// TODO implement vunmapbuf
void
vunmapbuf(struct buf *bp, vsize_t len)
{
}

// TODO implement cpu_lwp_setprivate
int
cpu_lwp_setprivate(lwp_t *l, void *addr)
{
	return 0;
}

/*
 * cpu_lwp_free is called from exit() to let machine-dependent
 * code free machine-dependent resources.  Note that this routine
 * must not block.
 */
void
cpu_lwp_free(struct lwp *l, int proc)
{
}

/*
 * cpu_lwp_free2 is called when an LWP is being reaped.
 * This routine may block.
 */
void
cpu_lwp_free2(struct lwp *l)
{
}

bool
cpu_intr_p(void)
{
	int idepth;

	kpreempt_disable();
	idepth = curcpu()->ci_idepth;
	kpreempt_enable();
	return (idepth >= 0);
}


/*
 * mm_md_physacc: check if given pa is accessible.
 */

// TODO: this is copy paste from i386, must be adapted for lm32
int
mm_md_physacc(paddr_t pa, vm_prot_t prot)
{
/*	extern phys_ram_seg_t mem_clusters[VM_PHYSSEG_MAX];
	extern int mem_cluster_cnt;
	int i;

	for (i = 0; i < mem_cluster_cnt; i++) {
		const phys_ram_seg_t *seg = &mem_clusters[i];
		paddr_t lstart = seg->start;

		if (lstart <= pa && pa - lstart <= seg->size) {
			return 0;
		}
	}
	return kauth_authorize_machdep(kauth_cred_get(),
	    KAUTH_MACHDEP_UNMANAGEDMEM, NULL, NULL, NULL, NULL);
*/
	return 0;
}

void
cpu_getmcontext(struct lwp *l, mcontext_t *mcp, unsigned int *flags)
{
	const struct trapframe *tf = l->l_md.md_utf;
	__greg_t *gr = mcp->__gregs;
	__greg_t ras_pc;

	gr[_REG_R1] = tf->tf_regs.r_regs[R1];
	gr[_REG_R2] = tf->tf_regs.r_regs[R2];
	gr[_REG_R3] = tf->tf_regs.r_regs[R3];
	gr[_REG_R4] = tf->tf_regs.r_regs[R4];
	gr[_REG_R5] = tf->tf_regs.r_regs[R5];
	gr[_REG_R6] = tf->tf_regs.r_regs[R6];
	gr[_REG_R7] = tf->tf_regs.r_regs[R7];
	gr[_REG_R8] = tf->tf_regs.r_regs[R8];
	gr[_REG_R9] = tf->tf_regs.r_regs[R9];
	gr[_REG_R10] = tf->tf_regs.r_regs[R10];
	gr[_REG_R11] = tf->tf_regs.r_regs[R11];
	gr[_REG_R12] = tf->tf_regs.r_regs[R12];
	gr[_REG_R13] = tf->tf_regs.r_regs[R13];
	gr[_REG_R14] = tf->tf_regs.r_regs[R14];
	gr[_REG_R15] = tf->tf_regs.r_regs[R15];
	gr[_REG_R16] = tf->tf_regs.r_regs[R16];
	gr[_REG_R17] = tf->tf_regs.r_regs[R17];
	gr[_REG_R18] = tf->tf_regs.r_regs[R18];
	gr[_REG_R19] = tf->tf_regs.r_regs[R19];
	gr[_REG_R20] = tf->tf_regs.r_regs[R20];
	gr[_REG_R21] = tf->tf_regs.r_regs[R21];
	gr[_REG_R22] = tf->tf_regs.r_regs[R22];
	gr[_REG_R23] = tf->tf_regs.r_regs[R23];
	gr[_REG_R24] = tf->tf_regs.r_regs[R24];
	gr[_REG_R25] = tf->tf_regs.r_regs[R25];
	gr[_REG_GP] = tf->tf_regs.r_regs[R_GP];
	gr[_REG_FP] = tf->tf_regs.r_regs[R_FP];
	gr[_REG_SP] = tf->tf_regs.r_regs[R_SP];
	gr[_REG_RA] = tf->tf_regs.r_regs[R_RA];
	gr[_REG_EA] = tf->tf_regs.r_regs[R_EA];
	gr[_REG_BA] = tf->tf_regs.r_regs[R_BA];

	gr[_REG_PC] = tf->tf_pc;

	if ((ras_pc = (__greg_t)ras_lookup(l->l_proc,
	    (void *) gr[_REG_PC])) != -1)
		gr[_REG_PC] = ras_pc;

}

int
cpu_setmcontext(struct lwp *l, const mcontext_t *mcp, unsigned int flags)
{
	struct trapframe *tf = l->l_md.md_utf;
	const __greg_t *gr = mcp->__gregs;
	struct proc *p = l->l_proc;
	int error;

	/* Restore register context, if any. */
	if ((flags & _UC_CPU) != 0) {
		error = cpu_mcontext_validate(l, mcp);
		if (error)
			return error;

		tf->tf_regs.r_regs[R1] = gr[_REG_R1];
		tf->tf_regs.r_regs[R2] = gr[_REG_R2];
		tf->tf_regs.r_regs[R3] = gr[_REG_R3];
		tf->tf_regs.r_regs[R4] = gr[_REG_R4];
		tf->tf_regs.r_regs[R5] = gr[_REG_R5];
		tf->tf_regs.r_regs[R6] = gr[_REG_R6];
		tf->tf_regs.r_regs[R7] = gr[_REG_R7];
		tf->tf_regs.r_regs[R8] = gr[_REG_R8];
		tf->tf_regs.r_regs[R9] = gr[_REG_R9];
		tf->tf_regs.r_regs[R10] = gr[_REG_R10];
		tf->tf_regs.r_regs[R11] = gr[_REG_R11];
		tf->tf_regs.r_regs[R12] = gr[_REG_R12];
		tf->tf_regs.r_regs[R13] = gr[_REG_R13];
		tf->tf_regs.r_regs[R14] = gr[_REG_R14];
		tf->tf_regs.r_regs[R15] = gr[_REG_R15];
		tf->tf_regs.r_regs[R16] = gr[_REG_R16];
		tf->tf_regs.r_regs[R17] = gr[_REG_R17];
		tf->tf_regs.r_regs[R18] = gr[_REG_R18];
		tf->tf_regs.r_regs[R19] = gr[_REG_R19];
		tf->tf_regs.r_regs[R20] = gr[_REG_R20];
		tf->tf_regs.r_regs[R21] = gr[_REG_R21];
		tf->tf_regs.r_regs[R22] = gr[_REG_R22];
		tf->tf_regs.r_regs[R23] = gr[_REG_R23];
		tf->tf_regs.r_regs[R24] = gr[_REG_R24];
		tf->tf_regs.r_regs[R25] = gr[_REG_R25];
		tf->tf_regs.r_regs[R_GP] = gr[_REG_GP];
		tf->tf_regs.r_regs[R_FP] = gr[_REG_FP];
		tf->tf_regs.r_regs[R_SP] = gr[_REG_SP];
		tf->tf_regs.r_regs[R_RA] = gr[_REG_RA];
		tf->tf_regs.r_regs[R_EA] = gr[_REG_EA];
		tf->tf_regs.r_regs[R_BA] = gr[_REG_BA];
	}

	mutex_enter(p->p_lock);
	if (flags & _UC_SETSTACK)
		l->l_sigstk.ss_flags |= SS_ONSTACK;
	if (flags & _UC_CLRSTACK)
		l->l_sigstk.ss_flags &= ~SS_ONSTACK;
	mutex_exit(p->p_lock);
	return (0);
}

// TODO: do we need any validation here?
int
cpu_mcontext_validate(struct lwp *l, const mcontext_t *mcp)
{
	return 0;
}

/* 
 * startlwp: start of a new LWP.
 */
void
startlwp(void *arg)
{
	ucontext_t *uc = arg;
	lwp_t *l = curlwp;
	int error;

	error = cpu_setmcontext(l, &uc->uc_mcontext, uc->uc_flags);
	KASSERT(error == 0);

	kmem_free(uc, sizeof(ucontext_t));
	userret(l);
}
