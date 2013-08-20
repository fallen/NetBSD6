#include <sys/param.h>
#include <sys/cpu.h>
#include <sys/ktrace.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/systm.h>
#include <sys/syscallvar.h>
#include <sys/syscall.h>

#include <uvm/uvm_extern.h>

#include <lm32/frame.h>
#include <lm32/pcb.h>
#include <lm32/reg.h>

void
child_return(void *arg)
{
	struct lwp * const l = arg;
	struct trapframe * const tf = &l->l_md.md_utf;

	tf->tf_regs.r_regs[R1] = 0; // set return value to 0 as the result of fork() for the child
	ktrsysret(SYS_fork, 0, 0);
}
