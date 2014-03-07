#include <lm32/cpu.h>
#include <sys/cpu.h>
#include <lm32/reg.h>
#include <sys/exec.h>
#include <lm32/pmap.h>

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

	regs = &l->l_md.md_utf->tf_regs;
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

void _do_real_tlb_miss_handling(unsigned long int, unsigned long int, unsigned long int);

/* During this function, TLBs are ON, we must take great care when manipulating pointers */
void _do_real_tlb_miss_handling(unsigned long int vpfn, unsigned long int vaddr, unsigned long int ea)
{
	struct pmap *map;
	struct cpu_info *ci;
  pmap_segtab_t *st;
  pt_entry_t *ptp;
  pt_entry_t pte;
  unsigned int psw;

  ci = curcpu();
  map = ci->ci_curpm;
  st = map->pm_segtab;
  ptp = st->seg_tab[vaddr >> SEGSHIFT];

  if (ptp == NULL)
    panic("[ptp %#lx] Trying to access non mapped address %#lx from PC=%#lx!\n", (unsigned long)ptp, vaddr, ea);

  pte = ptp[(vaddr & L2_MASK) >> PGSHIFT];

  if (pte == 0)
    panic("[pte] Trying to access non mapped address %#lx from PC=%#lx!\n", vaddr, ea);

  // FIXME: for now all pages are mapped read-write because tlb fault handler is dummy
  if (vpfn & 1) // if we came from a DTLB miss, we refresh DTLB
  {
    pte |= 1; // indicate we want to refresh DTLB
    pte &= ~(0x2); // clear the read-only bit from PTE
  }

  asm volatile("wcsr TLBPADDR, %0" :: "r"(pte) : );

  psw = PSW_DTLBE | PSW_ITLBE; /* clear *USR, EDTLBE and IDTLBE flags */
  asm volatile ("wcsr PSW, %0" :: "r"(psw) : );
  asm volatile(
               "mv ea, ra\n\t"
               "eret"
              );
}
