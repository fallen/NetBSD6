/*	$NetBSD: pmap.h,v 1.38 2011/06/20 21:45:16 matt Exp $	*/

#ifndef _LM32_PMAP_H_
#define _LM32_PMAP_H_
#include <sys/resource.h>

#if !defined(_LOCORE) && (defined(MODULAR) || defined(_MODULE))

__CTASSERT(sizeof(struct vm_page_md) == sizeof(uintptr_t)*5);

#endif /* !LOCORE && (MODULAR || _MODULE) */

#define	__PMAP_PTP_N	512	/* # of page table page maps 2GB. */
#define	PMAP_GROWKERNEL
#define PMAP_STEAL_MEMORY

struct pmap;

static __inline void
pmap_remove_all(struct pmap *pmap)
{
	/* Nothing. */
}

void tlbflush(void);
void pmap_load(void);
void pmap_virtual_space(vaddr_t *start, vaddr_t *end);
paddr_t pmap_phys_address(paddr_t cookie);
void pmap_bootstrap(void);

#define		pmap_resident_count(pmap)       ((pmap)->pm_stats.resident_count)
#define		pmap_wired_count(pmap)          ((pmap)->pm_stats.wired_count)
#define		pmap_update(pmap)               ((void)0)
#define	__HAVE_VM_PAGE_MD

struct pv_entry;
struct vm_page_md {
	SLIST_HEAD(, pv_entry) pvh_head;
	int pvh_flags;
};

#include <uvm/uvm_page.h>

/*
 * Pmap stuff
 */
struct pmap {
	pt_entry_t **pm_ptp;
	struct pmap_statistics	pm_stats;	/* pmap statistics */
	int pm_refcnt;
};

#define L2_SLOT_PTE	(KERNBASE/NBPD_L2-1) /* 767: for recursive PDP map */
#define L2_SLOT_KERN	(KERNBASE/NBPD_L2)   /* 768: start of kernel space */

#define pmap_pa2pte(a)			(a)
#define pmap_pte2pa(a)			((a) & PG_FRAME)
#define pmap_pte_set(p, n)		do { *(p) = (n); } while (0)
#define pmap_pte_flush()		/* nothing */
#define pmap_copy(DP,SP,D,L,S)		/* nothing */

#define	PVH_REFERENCED		1
#define	PVH_MODIFIED		2

/* MD pmap utils. */
pt_entry_t *__pmap_pte_lookup(pmap_t, vaddr_t);
bool __pmap_pte_load(pmap_t, vaddr_t, int);
pt_entry_t *__pmap_kpte_lookup(vaddr_t);

#define	VM_MDPAGE_INIT(pg)						\
do {									\
	struct vm_page_md *pvh = &(pg)->mdpage;				\
	SLIST_INIT(&pvh->pvh_head);					\
	pvh->pvh_flags = 0;						\
} while (/*CONSTCOND*/0)


#endif /* !_LM32_PMAP_H_ */
