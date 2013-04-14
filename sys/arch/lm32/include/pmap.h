/*	$NetBSD: pmap.h,v 1.38 2011/06/20 21:45:16 matt Exp $	*/

#ifndef _LM32_PMAP_H_
#define _LM32_PMAP_H_

#if !defined(_LOCORE) && (defined(MODULAR) || defined(_MODULE))

__CTASSERT(sizeof(struct vm_page_md) == sizeof(uintptr_t)*5);

#endif /* !LOCORE && (MODULAR || _MODULE) */

#define L2_SLOT_PTE	(KERNBASE/NBPD_L2-1) /* 767: for recursive PDP map */
#define L2_SLOT_KERN	(KERNBASE/NBPD_L2)   /* 768: start of kernel space */

#define pmap_pa2pte(a)			(a)
#define pmap_pte2pa(a)			((a) & PG_FRAME)
#define pmap_pte_set(p, n)		do { *(p) = (n); } while (0)
#define pmap_pte_flush()		/* nothing */

void tlbflush(void);
void pmap_load(void);

#endif /* !_LM32_PMAP_H_ */
