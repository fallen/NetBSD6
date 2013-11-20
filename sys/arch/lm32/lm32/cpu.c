#include <lm32/cpu.h>
#include <sys/cpu.h>
#include <lm32/reg.h>
#include <sys/exec.h>

void
cpu_intr_redistribute(void)
{

	/* XXX nothing */
}

u_int cpu_intr_count(struct cpu_info *ci)
{
	return ci->ci_nintrhand;
}

void cpu_offline_md(void)
{
	/* XXX nothing */
}


/*
 * Clear registers on exec
 */
void
setregs(struct lwp *l, struct exec_package *pack, vaddr_t stack)
{
//	struct pmap *pmap = vm_map_pmap(&l->l_proc->p_vmspace->vm_map);
	struct reg *regs;
	int i;

	regs = &l->l_md.md_utf.tf_regs;
	for (i = R1 ; i <= R27 ; i++)
		regs->r_regs[i] = 0;

	regs->r_pc = pack->ep_entry;
	regs->r_regs[R28] = stack; // R28: stack pointer
}


//TODO: use Milkymist SoC timer to provide more precise delay function?
void delay_func(unsigned int n)
{
	unsigned int i = n;
	while(i-- > 0);
}

void _do_real_tlb_miss_handling(void);

void _do_real_tlb_miss_handling(void)
{
	struct pmap *map;
	struct cpu_info *ci = (struct cpu_info *)((unsigned int)(curcpu() - 0xc0000000 + 0x40000000));

	map = ci->ci_pmap;
	// TODO FIXME: do something in here...
}
