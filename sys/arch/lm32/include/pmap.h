/*	$NetBSD: pmap.h,v 1.38 2011/06/20 21:45:16 matt Exp $	*/

#ifndef _LM32_PMAP_H_
#define _LM32_PMAP_H_
#include <sys/resource.h>
#include <uvm/uvm_page.h>

#if !defined(_LOCORE) && (defined(MODULAR) || defined(_MODULE))

__CTASSERT(sizeof(struct vm_page_md) == sizeof(uintptr_t)*5);

#endif /* !LOCORE && (MODULAR || _MODULE) */


/*
 * Pmap stuff
 */
struct pmap {
	pt_entry_t		*pm_ptab;	/* KVA of page table */
	u_int			pm_count;	/* pmap reference count */
	struct pmap_statistics	pm_stats;	/* pmap statistics */
	int pm_refcnt;
	int			pm_ptpages;	/* more stats: PT pages */
};

#define L2_SLOT_PTE	(KERNBASE/NBPD_L2-1) /* 767: for recursive PDP map */
#define L2_SLOT_KERN	(KERNBASE/NBPD_L2)   /* 768: start of kernel space */

#define pmap_pa2pte(a)			(a)
#define pmap_pte2pa(a)			((a) & PG_FRAME)
#define pmap_pte_set(p, n)		do { *(p) = (n); } while (0)
#define pmap_pte_flush()		/* nothing */

void tlbflush(void);
void pmap_load(void);
void pmap_reference(pmap_t pmap);

#endif /* !_LM32_PMAP_H_ */
