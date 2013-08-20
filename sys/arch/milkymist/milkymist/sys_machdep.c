#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/syscallargs.h>
#include <sys/cpu.h>
#include <uvm/uvm_extern.h>

int
sys_sysarch(struct lwp *l, const struct sys_sysarch_args *uap, register_t *retval)
{
	/*
	 * Currently no special system calls
	 */
	return (ENOSYS);
}
