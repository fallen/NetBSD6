#include <sys/param.h>
#include <sys/cpu.h>
#include <sys/ktrace.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/systm.h>
#include <sys/syscallvar.h>
#include <sys/syscall.h>
#include <sys/syscall_stats.h>
#include <uvm/uvm_extern.h>

#include <lm32/frame.h>
#include <lm32/pcb.h>
#include <lm32/reg.h>
#include <machine/userret.h>

void		syscall_intern(struct proc *);
static void	syscall(struct trapframe *);

void
child_return(void *arg)
{
	struct lwp * const l = arg;
	struct trapframe * const tf = &l->l_md.md_utf;

	tf->tf_regs.r_regs[R1] = 0; // set return value to 0 as the result of fork() for the child
	ktrsysret(SYS_fork, 0, 0);
}

/*
 * Process the tail end of a posix_spawn() for the child.
 */
void
cpu_spawn_return(struct lwp *l)
{
	userret(l);
}

void
syscall_intern(struct proc *p)
{

	p->p_md.md_syscall = syscall;
}

/*
 * syscall(frame):
 *	System call request from POSIX system call gate interface to kernel.
 *	Like trap(), argument is call by reference.
 */
static void
syscall(struct trapframe *frame)
{
	const struct sysent *callp;
	struct proc *p;
	struct lwp *l;
	int error;
	register_t code, rval[2];
	register_t args[2 + SYS_MAXSYSARGS];

	l = curlwp;
	p = l->l_proc;
	LWP_CACHE_CREDS(l, p);

	code = LM32_TF_R1(frame) & (SYS_NSYSENT - 1);
	callp = p->p_emul->e_sysent + code;

	SYSCALL_COUNT(syscall_counts, code);
	SYSCALL_TIME_SYS_ENTRY(l, syscall_times, code);

	if (callp->sy_argsize) {
		error = copyin((char *)frame->tf_regs.r_regs[R_SP] + sizeof(int), args,
			    callp->sy_argsize);
		if (__predict_false(error != 0))
			goto bad;
	}

	if (!__predict_false(p->p_trace_enabled)
	    || __predict_false(callp->sy_flags & SYCALL_INDIRECT)
	    || (error = trace_enter(code, args, callp->sy_narg)) == 0) {
		rval[0] = 0;
		rval[1] = 0;
		error = sy_call(callp, l, args, rval);
	}

	if (__predict_false(p->p_trace_enabled)
	    && !__predict_false(callp->sy_flags & SYCALL_INDIRECT)) {
		trace_exit(code, rval, error);
	}

	if (__predict_true(error == 0)) {
		LM32_TF_R1(frame) = rval[0];
		LM32_TF_R2(frame) = rval[1];
	} else {
		switch (error) {
		case ERESTART:
			/* nothing to do */
			break;
		case EJUSTRETURN:
			/* nothing to do */
			break;
		default:
		bad:
			LM32_TF_R1(frame) = error;
			break;
		}
	}

	SYSCALL_TIME_SYS_EXIT(l);
	userret(l);
}
