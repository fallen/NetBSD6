/*	$NetBSD: pmap.h,v 1.38 2011/06/20 21:45:16 matt Exp $	*/

#ifndef _LM32_PMAP_H_
#define _LM32_PMAP_H_
#include <sys/resource.h>
#include <sys/kcore.h>

#if !defined(_LOCORE) && (defined(MODULAR) || defined(_MODULE))

__CTASSERT(sizeof(struct vm_page_md) == sizeof(uintptr_t)*5);

#endif /* !LOCORE && (MODULAR || _MODULE) */

#define	__PMAP_PTP_N	512	/* # of page table page maps 2GB. */
#define	PMAP_TLB_NUM_PIDS		256
#define	KERNEL_PID	0
//#define	PMAP_GROWKERNEL
//#define PMAP_STEAL_MEMORY
#ifdef __PMAP_PRIVATE
#include <lm32/cpuvar.h>
#include <lm32/cpuset.h>
#endif

void tlbflush(void);
void pmap_load(void);
void pmap_virtual_space(vaddr_t *start, vaddr_t *end);
paddr_t pmap_phys_address(paddr_t cookie);
vaddr_t	pmap_md_map_poolpage(paddr_t, vsize_t);
void	pmap_md_unmap_poolpage(vaddr_t, vsize_t);
bool	pmap_md_direct_mapped_vaddr_p(vaddr_t);
paddr_t	pmap_md_direct_mapped_vaddr_to_paddr(vaddr_t);
bool	pmap_md_io_vaddr_p(vaddr_t);
void pmap_bootstrap(paddr_t, phys_ram_seg_t *);
void	pmap_md_init(void);
bool	pmap_md_tlb_check_entry(void *, vaddr_t, tlb_asid_t, pt_entry_t);
#define POOL_PHYSTOV(pa)  ((vaddr_t)(paddr_t)(pa) - 0x40000000 + 0xc0000000)
#define	POOL_VTOPHYS(va)	((paddr_t)(vaddr_t)(va) - 0xc0000000 + 0x40000000)

#ifdef __PMAP_PRIVATE
/*
 * Virtual Cache Alias helper routines.  Not a problem for LM32 CPUs.
 */
static inline bool
pmap_md_vca_add(struct vm_page *pg, vaddr_t va, pt_entry_t *nptep)
{
	return false;
}

static inline void
pmap_md_vca_remove(struct vm_page *pg, vaddr_t va)
{

}

#endif
#define		pmap_resident_count(pmap)       ((pmap)->pm_stats.resident_count)
#define		pmap_wired_count(pmap)          ((pmap)->pm_stats.wired_count)
#define	__HAVE_VM_PAGE_MD

#define	NPTEPG		(NBPG >> 2)
#define	SEGSHIFT	(PGSHIFT + PGSHIFT - 2)
#define SEGOFSET	((1 << SEGSHIFT) - 1)
#define	NBSEG		(NBPG*NPTEPG)
#define L2_MASK (0x003ff000)

#define	PMAP_INVALID_SEGTAB_ADDRESS	((pmap_segtab_t *)0xfeeddead)

#include <uvm/pmap/vmpagemd.h>

struct pv_entry;
#include <uvm/uvm_page.h>

#define L2_SLOT_PTE	(KERNBASE/NBPD_L2-1) /* 767: for recursive PDP map */
#define L2_SLOT_KERN	(KERNBASE/NBPD_L2)   /* 768: start of kernel space */

#define pmap_pa2pte(a)			(a)
#define pmap_pte2pa(a)			((a) & PG_FRAME)
#define pmap_pte_set(p, n)		do { *(p) = (n); } while (0)
#define pmap_pte_flush()		/* nothing */

#define	PVH_REFERENCED		1
#define	PVH_MODIFIED		2

/* MD pmap utils. */
pt_entry_t *__pmap_pte_lookup(pmap_t, vaddr_t);
bool __pmap_pte_load(pmap_t, vaddr_t, int);
pt_entry_t *__pmap_kpte_lookup(vaddr_t);

static inline void
pmap_md_vca_clean(struct vm_page *pg, vaddr_t va, int op)
{
}

static inline size_t
pmap_md_tlb_asid_max(void)
{
	return PMAP_TLB_NUM_PIDS - 1;
}
#include <uvm/pmap/pmap.h>
void	pmap_md_page_syncicache(struct vm_page *, __cpuset_t);

#endif /* !_LM32_PMAP_H_ */
