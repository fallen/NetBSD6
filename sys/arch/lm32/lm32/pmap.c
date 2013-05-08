/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/pool.h>
#include <sys/msgbuf.h>
#include <sys/socketvar.h>	/* XXX: for sock_loan_thresh */

#include <uvm/uvm.h>
#include <lm32/pmap.h>
#include <sys/types.h>
#include <lm32/cpu.h>
#include <sys/systm.h>
#include <uvm/uvm_map.h>
#include <uvm/uvm_extern.h>
#include <uvm/uvm_pmap.h>

#ifdef DEBUG
#define	STATIC
#else
#define	STATIC	static
#endif

#define	__PMAP_PTP_SHIFT	22
#define	__PMAP_PTP_TRUNC(va)						\
	(((va) + (1 << __PMAP_PTP_SHIFT) - 1) & ~((1 << __PMAP_PTP_SHIFT) - 1))
#define	__PMAP_PTP_PG_N		(PAGE_SIZE / sizeof(pt_entry_t))
#define	__PMAP_PTP_INDEX(va)	(((va) >> __PMAP_PTP_SHIFT) & (__PMAP_PTP_N - 1))
#define	__PMAP_PTP_OFSET(va)	((va >> PGSHIFT) & (__PMAP_PTP_PG_N - 1))

/* pmap pool */
STATIC struct pool __pmap_pmap_pool;
STATIC struct pool __pmap_pv_pool;
struct pmap __pmap_kernel;
struct pmap *const kernel_pmap_ptr = &__pmap_kernel;
#define	__pmap_pv_alloc()	pool_get(&__pmap_pv_pool, PR_NOWAIT)
#define	__pmap_pv_free(pv)	pool_put(&__pmap_pv_pool, (pv))
STATIC void __pmap_pv_enter(pmap_t, struct vm_page *, vaddr_t);
STATIC void __pmap_pv_remove(pmap_t, struct vm_page *, vaddr_t);
STATIC void *__pmap_pv_page_alloc(struct pool *, int);
STATIC void __pmap_pv_page_free(struct pool *, void *);
STATIC struct pool_allocator pmap_pv_page_allocator = {
	__pmap_pv_page_alloc, __pmap_pv_page_free, 0,
};

STATIC vaddr_t __pmap_kve;	/* VA of last kernel virtual */
paddr_t avail_start;		/* PA of first available physical page */
paddr_t avail_end;		/* PA of last available physical page */

/* For the fast tlb miss handler */
pt_entry_t **curptd;		/* p1 va of curlwp->...->pm_ptp */

struct pv_entry {
	struct pmap *pv_pmap;
	vaddr_t pv_va;
	SLIST_ENTRY(pv_entry) pv_link;
};

/* page table entry ops. */
STATIC pt_entry_t *__pmap_pte_alloc(pmap_t, vaddr_t);
/* pmap_enter util */
STATIC bool __pmap_map_change(pmap_t, vaddr_t, paddr_t, vm_prot_t,
    pt_entry_t);

void pmap_bootstrap(void)
{
	/* Steal msgbuf area */
	initmsgbuf((void *)uvm_pageboot_alloc(MSGBUFSIZE), MSGBUFSIZE);

	avail_start = ptoa(VM_PHYSMEM_PTR(0)->start);
	avail_end = ptoa(VM_PHYSMEM_PTR(vm_nphysseg - 1)->end);
	__pmap_kve = VM_MIN_KERNEL_ADDRESS;

	pmap_kernel()->pm_refcnt = 1;
	pmap_kernel()->pm_ptp = (pt_entry_t **)uvm_pageboot_alloc(PAGE_SIZE);
	memset(pmap_kernel()->pm_ptp, 0, PAGE_SIZE);

	/* Mask all interrupt */
	asm volatile("wcsr IM, r0"); // IM = 0;
	/* Enable MMU */
	lm32_mmu_start();
}

vaddr_t
pmap_steal_memory(vsize_t size, vaddr_t *vstart, vaddr_t *vend)
{
	struct vm_physseg *bank;
	int i, j, npage;
	paddr_t pa;
	vaddr_t va;

	KDASSERT(!uvm.page_init_done);

	size = round_page(size);
	npage = atop(size);

	bank = NULL;
	for (i = 0; i < vm_nphysseg; i++) {
		bank = VM_PHYSMEM_PTR(i);
		if (npage <= bank->avail_end - bank->avail_start)
			break;
	}
	KDASSERT(i != vm_nphysseg);
	KDASSERT(bank != NULL);

	/* Steal pages */
	pa = ptoa(bank->avail_start);
	bank->avail_start += npage;
	bank->start += npage;

	/* GC memory bank */
	if (bank->avail_start == bank->end) {
		/* Remove this segment from the list. */
		vm_nphysseg--;
		KDASSERT(vm_nphysseg > 0);
		for (j = i; i < vm_nphysseg; j++)
			VM_PHYSMEM_PTR_SWAP(j, j + 1);
	}

	va = pa;
	memset((void *)va, 0, size);

	return (va);
}

vaddr_t
pmap_growkernel(vaddr_t maxkvaddr)
{
	int i, n;

	if (maxkvaddr <= __pmap_kve)
		return (__pmap_kve);

	i = __PMAP_PTP_INDEX(__pmap_kve - VM_MIN_KERNEL_ADDRESS);
	__pmap_kve = __PMAP_PTP_TRUNC(maxkvaddr);
	n = __PMAP_PTP_INDEX(__pmap_kve - VM_MIN_KERNEL_ADDRESS);

	/* Allocate page table pages */
	for (;i < n; i++) {
		if (__pmap_kernel.pm_ptp[i] != NULL)
			continue;

		if (uvm.page_init_done) {
			struct vm_page *pg = uvm_pagealloc(NULL, 0, NULL,
			    UVM_PGA_USERESERVE | UVM_PGA_ZERO);
			if (pg == NULL)
				goto error;
			__pmap_kernel.pm_ptp[i] = (pt_entry_t *)
			    VM_PAGE_TO_PHYS(pg);
		} else {
			pt_entry_t *ptp = (pt_entry_t *)
			    uvm_pageboot_alloc(PAGE_SIZE);
			if (ptp == NULL)
				goto error;
			__pmap_kernel.pm_ptp[i] = ptp;
			memset(ptp, 0, PAGE_SIZE);
		}
	}

	return (__pmap_kve);
 error:
	panic("pmap_growkernel: out of memory.");
	/* NOTREACHED */
}

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

void pmap_destroy(pmap_t pmap)
{
	int i;

	if (--pmap->pm_refcnt > 0)
		return;

	/* Deallocate all page table page */
	for (i = 0; i < __PMAP_PTP_N; i++) {
		vaddr_t va = (vaddr_t)pmap->pm_ptp[i];
		if (va == 0)
			continue;
#ifdef DEBUG	/* Check no mapping exists. */
		{
			int j;
			pt_entry_t *pte = (pt_entry_t *)va;
			for (j = 0; j < __PMAP_PTP_PG_N; j++, pte++)
				KDASSERT(*pte == 0);
		}
#endif /* DEBUG */
		lm32_dtlb_invalidate_line((vaddr_t)va);
		/* invalidate CPU Data Cache not sure if this is needed */
		/* This is only needed if we can get cache aliasing */
//		lm32_dcache_invalidate();
		/* Free page table */
		uvm_pagefree(PHYS_TO_VM_PAGE(va));
	}
	/* Deallocate page table page holder */
	/* invalidate CPU Data Cache not sure if this is needed */
	/* This is only needed if we can get cache aliasing */
	lm32_dtlb_invalidate_line((vaddr_t)pmap->pm_ptp);
//	lm32_dcache_invalidate();
	uvm_pagefree(PHYS_TO_VM_PAGE((vaddr_t)pmap->pm_ptp));

	pool_put(&__pmap_pmap_pool, pmap);
}

pmap_t pmap_create(void)
{
	pmap_t pmap;

	pmap = pool_get(&__pmap_pmap_pool, PR_WAITOK);
	memset(pmap, 0, sizeof(struct pmap));
	pmap->pm_refcnt = 1;
	/* Allocate page table page holder (512 slot) */
	pmap->pm_ptp = (pt_entry_t **)
	    VM_PAGE_TO_PHYS(
		    uvm_pagealloc(NULL, 0, NULL,
			UVM_PGA_USERESERVE | UVM_PGA_ZERO));

	return (pmap);
}

void pmap_init(void)
{
	/* Initialize pmap module */
	pool_init(&__pmap_pmap_pool, sizeof(struct pmap), 0, 0, 0, "pmappl",
	    &pool_allocator_nointr, IPL_NONE);
	pool_init(&__pmap_pv_pool, sizeof(struct pv_entry), 0, 0, 0, "pvpl",
	    &pmap_pv_page_allocator, IPL_NONE);
	pool_setlowat(&__pmap_pv_pool, 16);
	pool_setlowat(&__pmap_pv_pool, 252);
}

paddr_t pmap_phys_address(paddr_t cookie)
{
	return (lm32_ptob(cookie));
}

void pmap_virtual_space(vaddr_t *start, vaddr_t *end)
{

	*start = VM_MIN_KERNEL_ADDRESS;
	*end = VM_MAX_KERNEL_ADDRESS;
}

/*
 * pv_entry pool allocator:
 *	void *__pmap_pv_page_alloc(struct pool *pool, int flags):
 *	void __pmap_pv_page_free(struct pool *pool, void *v):
 */
void * __pmap_pv_page_alloc(struct pool *pool, int flags)
{
	struct vm_page *pg;

	pg = uvm_pagealloc(NULL, 0, NULL, UVM_PGA_USERESERVE);
	if (pg == NULL)
		return (NULL);

	return ((void *)VM_PAGE_TO_PHYS(pg));
}

void __pmap_pv_page_free(struct pool *pool, void *v)
{
	vaddr_t va = (vaddr_t)v;

	/* Invalidate cache for next use of this page */
	/* Only needed if cache can have aliases */
//	lm32_icache_invalidate();
	uvm_pagefree(PHYS_TO_VM_PAGE(va));
}

void pmap_activate(struct lwp *l)
{
	pmap_t pmap = l->l_proc->p_vmspace->vm_map.pmap;

/* Doing lazy tlb updating ? 
	lm32_tlb_load_pmap(&pmap);
*/

	curptd = pmap->pm_ptp;
}

void pmap_deactivate(struct lwp *l)
{

	/* Nothing to do */
}

int pmap_enter(pmap_t pmap, vaddr_t va, paddr_t pa, vm_prot_t prot, u_int flags)
{
	struct vm_page *pg;
	struct vm_page_md *pvh;
	pt_entry_t entry, *pte;

	/* "flags" never exceed "prot" */
	KDASSERT(prot != 0 && ((flags & VM_PROT_ALL) & ~prot) == 0);

	pg = PHYS_TO_VM_PAGE(pa);
	entry = (pa & PG_FRAME);
	if (flags & PMAP_WIRED)
		entry |= PG_WIRED;

	if (pg != NULL) {	/* memory-space */
		pvh = VM_PAGE_TO_MD(pg);
		entry &= ~(PG_NOCACHE); /* always cached */

		/* Seed modified/reference tracking */
		if (flags & VM_PROT_WRITE) {
			entry |= PG_D;
			pvh->pvh_flags |= PVH_MODIFIED | PVH_REFERENCED;
		} else if (flags & VM_PROT_ALL) {
			pvh->pvh_flags |= PVH_REFERENCED;
		}

		/* Protection */
		if ((prot & VM_PROT_WRITE) && (pvh->pvh_flags & PVH_MODIFIED)) {
			entry &= ~(PG_RO);
		} else {
			entry |= (PG_RO);
		}

		/* Check for existing mapping */
		if (__pmap_map_change(pmap, va, pa, prot, entry))
			return (0);

		/* Add to physical-virtual map list of this page */
		__pmap_pv_enter(pmap, pg, va);

	} else {	/* bus-space (always uncached map) */
		if (prot & VM_PROT_WRITE)
		{
			entry &= ~(PG_RO);
			entry |= (PG_D);
		}
		else
			entry |= PG_RO;
	}

	/* Register to page table */
	pte = __pmap_pte_alloc(pmap, va);
	if (pte == NULL) {
		if (flags & PMAP_CANFAIL)
			return ENOMEM;
		panic("pmap_enter: cannot allocate pte");
	}

	*pte = entry;

	lm32_dtlb_update(va, entry);
	if (prot & VM_PROT_EXECUTE)
		lm32_itlb_update(va, entry);

/* No need to flush Instruction Cache IMO
	if (!SH_HAS_UNIFIED_CACHE &&
	    (prot == (VM_PROT_READ | VM_PROT_EXECUTE)))
		sh_icache_sync_range_index(va, PAGE_SIZE);
*/
	if (entry & PG_WIRED)
		pmap->pm_stats.wired_count++;
	pmap->pm_stats.resident_count++;

	return (0);
}

void pmap_remove(pmap_t pmap, vaddr_t sva, vaddr_t eva)
{
	struct vm_page *pg;
	pt_entry_t *pte, entry;
	vaddr_t va;

	KDASSERT((sva & PGOFSET) == 0);

	for (va = sva; va < eva; va += PAGE_SIZE) {
		if ((pte = __pmap_pte_lookup(pmap, va)) == NULL ||
		    (entry = *pte) == 0)
			continue;

		if ((pg = PHYS_TO_VM_PAGE(entry & PG_FRAME)) != NULL)
			__pmap_pv_remove(pmap, pg, va);

		if (entry & PG_WIRED)
			pmap->pm_stats.wired_count--;
		pmap->pm_stats.resident_count--;
		*pte = 0;

		lm32_tlb_invalidate_line(va);
	}
}

void pmap_protect(pmap_t pmap, vaddr_t sva, vaddr_t eva, vm_prot_t prot)
{
	bool prot_exec = prot & VM_PROT_EXECUTE;
	pt_entry_t *pte, entry, protbits;
	vaddr_t va;

	sva = trunc_page(sva);

	if ((prot & VM_PROT_READ) == VM_PROT_NONE) {
		pmap_remove(pmap, sva, eva);
		return;
	}

	switch (prot) {
	default:
		panic("pmap_protect: invalid protection mode %x", prot);
		/* NOTREACHED */
	case VM_PROT_READ:
		/* FALLTHROUGH */
	case VM_PROT_READ | VM_PROT_EXECUTE:
		protbits = PG_RO;
		break;
	case VM_PROT_READ | VM_PROT_WRITE:
		/* FALLTHROUGH */
	case VM_PROT_ALL:
		break;
	}

	for (va = sva; va < eva; va += PAGE_SIZE) {

		if (((pte = __pmap_pte_lookup(pmap, va)) == NULL) ||
		    (entry = *pte) == 0)
			continue;

/* I think this is not needed
		if (LM32_HAS_VIRTUAL_ALIAS && (entry & PG_D)) {
			if (!SH_HAS_UNIFIED_CACHE && (prot & VM_PROT_EXECUTE))
				lm32_icache_invalidate();
			else
				lm32_dcache_invalidate();
		}
*/
		entry = (entry & ~PG_FRAME) | protbits;
		*pte = entry;

		lm32_dtlb_update(va, entry);
		if (prot_exec)
			lm32_itlb_update(va, entry);
		else
			lm32_itlb_invalidate_line(va);
	}
}

void pmap_unwire(pmap_t pmap, vaddr_t va)
{
	pt_entry_t *pte, entry;

	if ((pte = __pmap_pte_lookup(pmap, va)) == NULL ||
	    (entry = *pte) == 0 ||
	    (entry & PG_WIRED) == 0)
		return;

	*pte = entry & ~PG_WIRED;
	pmap->pm_stats.wired_count--;
}

bool pmap_extract(pmap_t pmap, vaddr_t va, paddr_t *pap)
{
	pt_entry_t *pte;

	pte = __pmap_pte_lookup(pmap, va);
	if (pte == NULL || *pte == 0)
		return (false);

	if (pap != NULL)
		*pap = (*pte & PG_FRAME) | (va & PGOFSET);

	return (true);
}

void pmap_kenter_pa(vaddr_t va, paddr_t pa, vm_prot_t prot, u_int flags)
{
	pt_entry_t *pte, entry;

	KDASSERT((va & PGOFSET) == 0);
	KDASSERT(va >= VM_MIN_KERNEL_ADDRESS && va < VM_MAX_KERNEL_ADDRESS);

	entry = (pa & PG_FRAME);
	if (prot & VM_PROT_WRITE)
		entry |= PG_D;
	else
		entry |= PG_RO;

	pte = __pmap_kpte_lookup(va);

	KDASSERT(*pte == 0);
	*pte = entry;

	if (prot & VM_PROT_EXECUTE)
		lm32_itlb_update(va, entry);
	else
		lm32_itlb_invalidate_line(va);

	lm32_dtlb_update(va, entry);
}

void pmap_kremove(vaddr_t va, vsize_t len)
{
	pt_entry_t *pte;
	vaddr_t eva = va + len;

	KDASSERT((va & PGOFSET) == 0);
	KDASSERT((len & PGOFSET) == 0);
	KDASSERT(va >= VM_MIN_KERNEL_ADDRESS && eva <= VM_MAX_KERNEL_ADDRESS);

	for (; va < eva; va += PAGE_SIZE) {
		pte = __pmap_kpte_lookup(va);
		KDASSERT(pte != NULL);
		if (*pte == 0)
			continue;
/* I think this is not needed
		if (SH_HAS_VIRTUAL_ALIAS && PHYS_TO_VM_PAGE(*pte & PG_FRAME))
			sh_dcache_wbinv_range(va, PAGE_SIZE);
*/
		*pte = 0;

		lm32_tlb_invalidate_line(va);
	}
}

void pmap_copy_page(paddr_t src, paddr_t dst)
{
	memcpy((void *)dst, (void *)src, PAGE_SIZE);
}

void pmap_zero_page(paddr_t phys)
{
	memset((void *)phys, 0, PAGE_SIZE);
}

void pmap_page_protect(struct vm_page *pg, vm_prot_t prot)
{
	struct vm_page_md *pvh = VM_PAGE_TO_MD(pg);
	struct pv_entry *pv;
	struct pmap *pmap;
	vaddr_t va;
	int s;

	switch (prot) {
	case VM_PROT_READ | VM_PROT_WRITE:
		/* FALLTHROUGH */
	case VM_PROT_ALL:
		break;

	case VM_PROT_READ:
		/* FALLTHROUGH */
	case VM_PROT_READ | VM_PROT_EXECUTE:
		s = splvm();
		SLIST_FOREACH(pv, &pvh->pvh_head, pv_link) {
			pmap = pv->pv_pmap;
			va = pv->pv_va;

			KDASSERT(pmap);
			pmap_protect(pmap, va, va + PAGE_SIZE, prot);
		}
		splx(s);
		break;

	default:
		/* Remove all */
		s = splvm();
		while ((pv = SLIST_FIRST(&pvh->pvh_head)) != NULL) {
			va = pv->pv_va;
			pmap_remove(pv->pv_pmap, va, va + PAGE_SIZE);
		}
		splx(s);
	}
}

bool pmap_clear_modify(struct vm_page *pg)
{
	struct vm_page_md *pvh = VM_PAGE_TO_MD(pg);
	struct pv_entry *pv;
	struct pmap *pmap;
	pt_entry_t *pte, entry;
	bool modified;
	vaddr_t va;
	int s;

	modified = pvh->pvh_flags & PVH_MODIFIED;
	if (!modified)
		return (false);

	pvh->pvh_flags &= ~PVH_MODIFIED;

	s = splvm();
	if (SLIST_EMPTY(&pvh->pvh_head)) {/* no map on this page */
		splx(s);
		return (true);
	}

	/* Write-back and invalidate TLB entry */
//	if (!SH_HAS_VIRTUAL_ALIAS && SH_HAS_WRITEBACK_CACHE)
		lm32_dcache_invalidate();

	SLIST_FOREACH(pv, &pvh->pvh_head, pv_link) {
		pmap = pv->pv_pmap;
		va = pv->pv_va;
		if ((pte = __pmap_pte_lookup(pmap, va)) == NULL)
			continue;
		entry = *pte;
		if ((entry & PG_D) == 0)
			continue;
// I think this is not needed
//		if (SH_HAS_VIRTUAL_ALIAS)
//			sh_dcache_wbinv_range_index(va, PAGE_SIZE);

		*pte = entry & ~PG_D;
		lm32_tlb_invalidate_line(va);
	}
	splx(s);

	return (true);
}

bool pmap_clear_reference(struct vm_page *pg)
{
	struct vm_page_md *pvh = VM_PAGE_TO_MD(pg);
	struct pv_entry *pv;
	pt_entry_t *pte;
	pmap_t pmap;
	vaddr_t va;
	int s;

	if ((pvh->pvh_flags & PVH_REFERENCED) == 0)
		return (false);

	pvh->pvh_flags &= ~PVH_REFERENCED;

	s = splvm();
	/* Restart reference bit emulation */
	SLIST_FOREACH(pv, &pvh->pvh_head, pv_link) {
		pmap = pv->pv_pmap;
		va = pv->pv_va;

		if ((pte = __pmap_pte_lookup(pmap, va)) == NULL)
			continue;

		lm32_tlb_invalidate_line(va);
	}
	splx(s);

	return (true);
}

bool pmap_is_modified(struct vm_page *pg)
{
	struct vm_page_md *pvh = VM_PAGE_TO_MD(pg);

	return ((pvh->pvh_flags & PVH_MODIFIED) ? true : false);
}

bool pmap_is_referenced(struct vm_page *pg)
{
	struct vm_page_md *pvh = VM_PAGE_TO_MD(pg);

	return ((pvh->pvh_flags & PVH_REFERENCED) ? true : false);
}

/*
 * pt_entry_t *__pmap_pte_lookup(pmap_t pmap, vaddr_t va):
 *	lookup page table entry, if not allocated, returns NULL.
 */
pt_entry_t *
__pmap_pte_lookup(pmap_t pmap, vaddr_t va)
{
	pt_entry_t *ptp;

	/* Lookup page table page */
	ptp = pmap->pm_ptp[__PMAP_PTP_INDEX(va)];
	if (ptp == NULL)
		return (NULL);

	return (ptp + __PMAP_PTP_OFSET(va));
}

/*
 * bool __pmap_map_change(pmap_t pmap, vaddr_t va, paddr_t pa,
 *     vm_prot_t prot, pt_entry_t entry):
 *	Handle the situation that pmap_enter() is called to enter a
 *	mapping at a virtual address for which a mapping already
 *	exists.
 */
bool
__pmap_map_change(pmap_t pmap, vaddr_t va, paddr_t pa, vm_prot_t prot,
    pt_entry_t entry)
{
	pt_entry_t *pte, oentry;
	vaddr_t eva = va + PAGE_SIZE;

	if ((pte = __pmap_pte_lookup(pmap, va)) == NULL ||
	    ((oentry = *pte) == 0))
		return (false);		/* no mapping exists. */

	if (pa != (oentry & PG_FRAME)) {
		/* Enter a mapping at a mapping to another physical page. */
		pmap_remove(pmap, va, eva);
		return (false);
	}

	/* Pre-existing mapping */

	/* Protection change. */
	if ((oentry & PG_PR_MASK) != (entry & PG_PR_MASK))
		pmap_protect(pmap, va, eva, prot);

	/* Wired change */
	if (oentry & PG_WIRED) {
		if (!(entry & PG_WIRED)) {
			/* wired -> unwired */
			*pte = entry;
			/* "wired" is software bits. no need to update TLB */
			pmap->pm_stats.wired_count--;
		}
	} else if (entry & PG_WIRED) {
		/* unwired -> wired. make sure to reflect "flags" */
		pmap_remove(pmap, va, eva);
		return (false);
	}

	return (true);	/* mapping was changed. */
}

/*
 * pt_entry_t *__pmap_kpte_lookup(vaddr_t va):
 *	kernel virtual only version of __pmap_pte_lookup().
 */
pt_entry_t *
__pmap_kpte_lookup(vaddr_t va)
{
	pt_entry_t *ptp;

	ptp = __pmap_kernel.pm_ptp[__PMAP_PTP_INDEX(va-VM_MIN_KERNEL_ADDRESS)];
	if (ptp == NULL)
		return NULL;

	return (ptp + __PMAP_PTP_OFSET(va));
}

/*
 * void __pmap_pv_remove(pmap_t pmap, struct vm_page *pg, vaddr_t vaddr):
 *	Remove physical-virtual map from vm_page.
 */
void
__pmap_pv_remove(pmap_t pmap, struct vm_page *pg, vaddr_t vaddr)
{
	struct vm_page_md *pvh;
	struct pv_entry *pv;
	int s;

	s = splvm();
	pvh = VM_PAGE_TO_MD(pg);
	SLIST_FOREACH(pv, &pvh->pvh_head, pv_link) {
		if (pv->pv_pmap == pmap && pv->pv_va == vaddr) {
// FIXME
/* is this needed ??? */
			if 
//			    (SH_HAS_VIRTUAL_ALIAS ||
//			    (SH_HAS_WRITEBACK_CACHE &&
				(pvh->pvh_flags & PVH_MODIFIED)
//			)) 
			{
				/*
				 * Always use index ops. since I don't want to
				 * worry about address space.
				 */
				lm32_dcache_invalidate();
			}

			SLIST_REMOVE(&pvh->pvh_head, pv, pv_entry, pv_link);
			__pmap_pv_free(pv);
			break;
		}
	}
#ifdef DEBUG
	/* Check duplicated map. */
	SLIST_FOREACH(pv, &pvh->pvh_head, pv_link)
	    KDASSERT(!(pv->pv_pmap == pmap && pv->pv_va == vaddr));
#endif
	splx(s);
}

/*
 * void __pmap_pv_enter(pmap_t pmap, struct vm_page *pg, vaddr_t vaddr):
 *	Insert physical-virtual map to vm_page.
 *	Assume pre-existed mapping is already removed.
 */
void
__pmap_pv_enter(pmap_t pmap, struct vm_page *pg, vaddr_t va)
{
	struct vm_page_md *pvh;
	struct pv_entry *pv;
	int s;

	s = splvm();
//	if (SH_HAS_VIRTUAL_ALIAS) {
	/*
	 * Remove all other mappings on this physical page
	 * which have different virtual cache indexes to
	 * avoid virtual cache aliases.
	 *
	 * XXX We should also handle shared mappings which
	 * XXX have different virtual cache indexes by
	 * XXX mapping them uncached (like arm and mips do).
	 */
/*again:
	pvh = VM_PAGE_TO_MD(pg);
	SLIST_FOREACH(pv, &pvh->pvh_head, pv_link) {
		if (lm32_cache_indexof(va) !=
		    lm32_cache_indexof(pv->pv_va)) {
			pmap_remove(pv->pv_pmap, pv->pv_va,
			    pv->pv_va + PAGE_SIZE);
			goto again;
		}
	}
//	}
*/
	/* Register pv map */
	pvh = VM_PAGE_TO_MD(pg);
	pv = __pmap_pv_alloc();
	pv->pv_pmap = pmap;
	pv->pv_va = va;

	SLIST_INSERT_HEAD(&pvh->pvh_head, pv, pv_link);
	splx(s);
}

/*
 * pt_entry_t __pmap_pte_alloc(pmap_t pmap, vaddr_t va):
 *	lookup page table entry. if found returns it, else allocate it.
 *	page table is accessed via P1.
 */
pt_entry_t *
__pmap_pte_alloc(pmap_t pmap, vaddr_t va)
{
	struct vm_page *pg;
	pt_entry_t *ptp, *pte;

	if ((pte = __pmap_pte_lookup(pmap, va)) != NULL)
		return (pte);

	/* Allocate page table (not managed page) */
	pg = uvm_pagealloc(NULL, 0, NULL, UVM_PGA_USERESERVE | UVM_PGA_ZERO);
	if (pg == NULL)
		return NULL;

	ptp = (pt_entry_t *)VM_PAGE_TO_PHYS(pg);
	pmap->pm_ptp[__PMAP_PTP_INDEX(va)] = ptp;

	return (ptp + __PMAP_PTP_OFSET(va));
}
