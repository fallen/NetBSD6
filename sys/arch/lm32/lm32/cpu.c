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

void _do_real_tlb_miss_handling(unsigned long int, unsigned long int);

/* During this function, TLBs are ON, we must take great care when manipulating pointers */
void _do_real_tlb_miss_handling(unsigned long int vpfn, unsigned long int vaddr)
{
	struct pmap *map;
	struct cpu_info *ci = curcpu();
  pmap_segtab_t *st;
  pt_entry_t *ptp;
  pt_entry_t pte;
  unsigned int psw;
  unsigned int paddr;

  asm volatile("rcsr %0, PSW" : "=r"(psw) :: );

  if (!(psw & PSW_EUSR))
  {
    if ( (vaddr < VM_MIN_KERNEL_ADDRESS + IOM_RAM_SIZE + (1 << PGSHIFT)-1 ) && (vaddr >= VM_MIN_KERNEL_ADDRESS) )
    {
      paddr = vpfn - VM_MIN_KERNEL_ADDRESS + IOM_RAM_BEGIN;
      asm volatile("wcsr TLBPADDR, %0" :: "r"(paddr) : );
      goto return_to_exception_handler;
    }
  }

  map = ci->ci_curpm;
  st = map->pm_segtab;
  ptp = st->seg_tab[vaddr >> SEGSHIFT];
  if (ptp == NULL)
    panic("[ptp] non mapped address !\n");

  pte = ptp[vaddr >> PGSHIFT];

  if (pte == 0)
    panic("[pte] non mapped address !\n");

  asm volatile("wcsr TLBPADDR, %0" :: "r"(pte) : );

return_to_exception_handler:
  psw = PSW_DTLBE | PSW_ITLBE; /* clear *USR, EDTLBE and IDTLBE flags */
  asm volatile(
               "mv ea, ra\n\t"
               "eret"
              );
}
