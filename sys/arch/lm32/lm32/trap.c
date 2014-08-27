/*
 * COPYRIGHT (C) 2014 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/param.h>
#include <uvm/uvm.h>

void lm32_trap(unsigned int vaddr, struct trapframe *tf, unsigned int return_address, vm_prot_t prot);

static inline struct vm_map *
get_faultmap(const struct trapframe * const tf)
{
	return (tf->tf_psw & PSW_EUSR)
	    ? &curlwp->l_proc->p_vmspace->vm_map
	    : kernel_map;
}

/*
 * We could use pmap_pte_lookup but this slightly faster since we already
 * the segtab pointers in cpu_info.
 */
static inline pt_entry_t *
trap_pte_lookup(struct trapframe *tf, vaddr_t va)
{
	pmap_segtab_t * stp;

	if (tf->tf_psw & PSW_EUSR)
		 stp = curcpu()->ci_pmap_user_segtab;
	else
		 stp = curcpu()->ci_pmap_kern_segtab;

	if (__predict_false(stp == NULL))
		return NULL;
	pt_entry_t * const ptep = stp->seg_tab[va >> SEGSHIFT];
	if (__predict_false(ptep == NULL))
		return NULL;
	return ptep + ((va & SEGOFSET) >> PAGE_SHIFT);
}

void lm32_trap(unsigned int vaddr, struct trapframe *tf, unsigned int return_address, vm_prot_t prot)
{
  int ret;
  unsigned int psw;
  struct trapframe *virt_tf = (struct trapframe *)kern_phy_to_virt_ramwindow(tf);
  struct vm_map * const faultmap = get_faultmap(virt_tf);

  if ((unsigned int *)vaddr == NULL)
    panic("NULL pointer dereferencement");

  if (prot & VM_PROT_EXECUTE)
  {
	pt_entry_t * ptep = trap_pte_lookup(virt_tf, vaddr & PG_FRAME);

	if (ptep == NULL)
	{
		ret = uvm_fault(kernel_map, vaddr, prot);
		if (ret != 0)
			ret = uvm_fault(&curlwp->l_proc->p_vmspace->vm_map, vaddr, VM_PROT_READ);
		ptep = trap_pte_lookup(virt_tf, vaddr & PG_FRAME);
	}
	KASSERT(ptep != NULL);
	pt_entry_t pte = *ptep;
	if ((pte & PTE_UNSYNCED) == PTE_UNSYNCED) {
		const paddr_t pa = pte_to_paddr(pte);
		struct vm_page * const pg = PHYS_TO_VM_PAGE(pa);
		KASSERT(pg);
		struct vm_page_md * const mdpg = VM_PAGE_TO_MD(pg);

		if (!VM_PAGEMD_EXECPAGE_P(mdpg)) {
			lm32_icache_invalidate();
			pmap_page_set_attributes(mdpg, VM_PAGEMD_EXECPAGE);
		}
		pte &= ~PTE_UNSYNCED;
		pte |= PTE_xX;
		*ptep = pte;

		pmap_tlb_update_addr(faultmap->pmap, vaddr & PG_FRAME,
		    pte, 0);
		goto exit;
	}
  }

  ret = uvm_fault(kernel_map, vaddr, prot);

  if (ret != 0)
  {
    ret = uvm_fault(&curlwp->l_proc->p_vmspace->vm_map, vaddr, VM_PROT_READ);
    if (ret != 0)
      panic("cannot resolve page fault");
  }
exit:
  asm volatile("rcsr %0, PSW" : "=r"(psw) :: );

  psw &= ~(PSW_IE_EIE | PSW_EDTLBE | PSW_EITLBE);

  asm volatile("wcsr PSW, %0" :: "r"(psw) : );

  asm volatile("mv ea, %0\n\t"
               "eret" :: "r"(return_address) : );
}

