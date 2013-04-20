/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/pmap.h>
#include <sys/types.h>
#include <lm32/cpu.h>
#include <sys/systm.h>
#include <uvm/uvm_map.h>
#include <uvm/uvm_extern.h>
void tlbflush(void)
{
	/* flush DTLB */
	asm volatile("xor r11, r11, r11\n\t"
		     "ori r11, r11, 0x3\n\t"
		     "wcsr tlbvaddr, r11" ::: "r11");	

	/* flush ITLB */
	asm volatile("xor r11, r11, r11\n\t"
		     "ori r11, r11, 0x2\n\t"
		     "wcsr tlbvaddr, r11" ::: "r11");	
}

void
pmap_reference(pmap_t pmap)
{

	pmap->pm_refcnt++;
}

/*
 * pmap_load: perform the actual pmap switch
 *
 * Ensures that the current process' pmap is loaded on the current CPU's
 * MMU and that there are no stale TLB entries.
 *
 * => The caller should disable kernel preemption or do check-and-retry
 *    to prevent a preemption from undoing our efforts.
 * => This function may block.
 */
void
pmap_load(void)
{
	struct cpu_info *ci;
	struct pmap *pmap, *oldpmap;
	struct lwp *l;
	struct pcb *pcb;
	uint64_t ncsw;

	kpreempt_disable();
 retry:
	ci = curcpu();
	if (!ci->ci_want_pmapload) {
		kpreempt_enable();
		return;
	}
	l = ci->ci_curlwp;
	ncsw = l->l_ncsw;

	/* should be able to take ipis. */
	KASSERT(ci->ci_ilevel < IPL_HIGH); 

	KASSERT(l != NULL);
	pmap = vm_map_pmap(&l->l_proc->p_vmspace->vm_map);
	KASSERT(pmap != pmap_kernel());
	oldpmap = ci->ci_pmap;
	pcb = lwp_getpcb(l);

	if (pmap == oldpmap) {
		ci->ci_want_pmapload = 0;
		kpreempt_enable();
		return;
	}

	/*
	 * Acquire a reference to the new pmap and perform the switch.
	 */

	pmap_reference(pmap);

	/*
	 * Mark the pmap in use by this CPU.  Again, we must synchronize
	 * with TLB shootdown interrupts, so set the state VALID first,
	 * then register us for shootdown events on this pmap.
	 */
	ci->ci_pmap = pmap;


	u_int gen = uvm_emap_gen_return();
	cpu_load_pmap(pmap, oldpmap);
	uvm_emap_update(gen);

	ci->ci_want_pmapload = 0;

	/*
	 * we're now running with the new pmap.  drop the reference
	 * to the old pmap.  if we block, we need to go around again.
	 */

	pmap_destroy(oldpmap);
	if (l->l_ncsw != ncsw) {
		goto retry;
	}

	kpreempt_enable();
}

