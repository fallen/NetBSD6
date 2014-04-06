/*	$NetBSD: $	*/

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/cpu.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/systm.h>

#include <uvm/uvm_extern.h>

#include <lm32/pcb.h>
#include <lm32/psl.h>
#include <lm32/reg.h>

int
process_set_pc(struct lwp *l, void *addr)
{
	return 0;
}

int
process_sstep(struct lwp *l, int sstep)
{
	return 0;
}

int
process_read_regs(struct lwp *l, struct reg *regs)
{
	return 0;
}

int
process_write_regs(struct lwp *l, const struct reg *regs)
{
	return 0;
}
