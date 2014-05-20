/*	$NetBSD: $	*/

/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */
#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/pool.h>
#include <sys/msgbuf.h>
#include <sys/socketvar.h>	/* XXX: for sock_loan_thresh */
#include <sys/buf.h>

#include <uvm/uvm.h>
#include <lm32/pmap.h>
#include <sys/types.h>
#include <lm32/cpu.h>
#include <sys/systm.h>
#include <uvm/uvm_map.h>
#include <uvm/uvm_extern.h>
#include <uvm/uvm_pmap.h>

#include <machine/uart.h>

/*
 * Initialize the kernel pmap.
 */
#ifdef MULTIPROCESSOR
#define	PMAP_SIZE	offsetof(struct pmap, pm_pai[MAXCPUS])
#else
#define	PMAP_SIZE	sizeof(struct pmap)
#endif

CTASSERT(sizeof(pmap_segtab_t) == NBPG);

pmap_segtab_t pmap_kernel_segtab;

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

struct pmap __pmap_kernel;
#define	__pmap_pv_alloc()	pool_get(&__pmap_pv_pool, PR_NOWAIT)
#define	__pmap_pv_free(pv)	pool_put(&__pmap_pv_pool, (pv))

/* For the fast tlb miss handler */
pt_entry_t **curptd;		/* p1 va of curlwp->...->pm_ptp */

void dcache_zero_page(vaddr_t va);
void dcache_zero_page(vaddr_t va)
{
  int i;
  for (i = 0 ; i < (NBPG >> 2) ; i++)
    *((unsigned int *)va + i) = 0;
}

void
pmap_zero_page(paddr_t pa)
{
	vaddr_t va = pmap_md_map_poolpage(pa, NBPG);
	dcache_zero_page(va);

	KASSERT(!VM_PAGEMD_EXECPAGE_P(VM_PAGE_TO_MD(PHYS_TO_VM_PAGE(pa))));
	pmap_md_unmap_poolpage(va, NBPG);
}

void
pmap_copy_page(paddr_t src, paddr_t dst)
{
  vaddr_t vsrc;
  vaddr_t vdst;
  int i;

  vsrc = pmap_md_map_poolpage(src, NBPG);
  vdst = pmap_md_map_poolpage(dst, NBPG);

  for (i = 0 ; i < (NBPG >> 2) ; i++)
    *((unsigned int *)vdst + i) = *((unsigned int *)vsrc + i);

	pmap_md_unmap_poolpage(vsrc, NBPG);
	pmap_md_unmap_poolpage(vdst, NBPG);
}

struct vm_page *
pmap_md_alloc_poolpage(int flags)
{
	/*
	 * Any managed page works for us.
	 */
	return uvm_pagealloc(NULL, 0, NULL, flags);
}

void tlb_set_asid(tlb_asid_t);

void _dtlb_miss_handler(void);
void _fake_dtlb_miss_handler(void);
void _real_tlb_miss_handler(void);
void _itlb_miss_handler(void);

#define IOM_RAM_BEGIN (0x40000000)

void pmap_bootstrap(paddr_t kernend, phys_ram_seg_t *avail)
{
//	vaddr_t msgbuf;
  pmap_segtab_t * const stp = &pmap_kernel_segtab;
	/* Steal msgbuf area */


	printf("pmap_bootstrap();\n");

	/* Load memory to UVM */
	physmem = atop(avail[0].size);

  pmap_kernel()->pm_segtab = stp;
  curcpu()->ci_pmap_kern_segtab = stp;

  printf("kernend == %#lx\n", kernend);
  printf("vm_physmem[0].start == %#lx\n", vm_physmem[0].start);
  printf("vm_physmem[0].end == %#lx\n", vm_physmem[0].end);

  pmap_limits.avail_start = vm_physmem[0].start << PGSHIFT;
  pmap_limits.avail_end = vm_physmem[0].end << PGSHIFT;
  kmeminit_nkmempages();

	/* Get size of buffer cache and set an upper limit */
	buf_setvalimit((VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS) / 8);
	vsize_t bufsz = buf_memcalc();
	buf_setvalimit(bufsz);

	vsize_t kv_nsegtabs = pmap_round_seg(VM_PHYS_SIZE
	    + (ubc_nwins << ubc_winshift)
	    + bufsz
	    + 16 * NCARGS
	    + pager_map_size
	    + maxproc * USPACE
#ifdef SYSVSHM
	    + NBPG * shminfo.shmall
#endif
	    + NBPG * nkmempages) >> SEGSHIFT;
//  printf("kv_nsegtabs == %#lx\n", kv_nsegtabs);

	/*
	 * Initialize `FYI' variables.	Note we're relying on
	 * the fact that BSEARCH sorts the vm_physmem[] array
	 * for us.  Must do this before uvm_pageboot_alloc()
	 * can be called.
	 */
	pmap_limits.avail_start = vm_physmem[0].start << PGSHIFT;
	pmap_limits.avail_end = vm_physmem[0].end << PGSHIFT;
//  printf("pmap_limist.avail_start == %#lx\n", pmap_limits.avail_start);
//  printf("pmap_limist.avail_end == %#lx\n", pmap_limits.avail_end);
	const size_t max_nsegtabs =
	    (pmap_round_seg(VM_MAX_KERNEL_ADDRESS)
		- pmap_trunc_seg(VM_MIN_KERNEL_ADDRESS)) / NBSEG;
	if (kv_nsegtabs >= max_nsegtabs) {
		pmap_limits.virtual_end = VM_MAX_KERNEL_ADDRESS;
		kv_nsegtabs = max_nsegtabs;
	} else {
		pmap_limits.virtual_end = VM_MIN_KERNEL_ADDRESS
		    + kv_nsegtabs * NBSEG;
	}

	/*
	 * Now actually allocate the kernel PTE array (must be done
	 * after virtual_end is initialized).
	 */
//  printf("avail[0].start == %#llx\n", avail[0].start);
	const vaddr_t kv_segtabs = kern_phy_to_virt(avail[0].start);
//	KASSERT(kv_segtabs == kernend);
	KASSERT(avail[0].size >= NBPG * kv_nsegtabs);
//	printf(" kv_nsegtabs=%#"PRIxVSIZE, kv_nsegtabs);
//	printf(" kv_segtabs=%#"PRIxVADDR"\n", kv_segtabs);
	avail[0].start += NBPG * kv_nsegtabs;
	avail[0].size -= NBPG * kv_nsegtabs;
	kernend += NBPG * kv_nsegtabs;
	/*
	 * Initialize the kernel's two-level page level.  This only wastes
	 * an extra page for the segment table and allows the user/kernel
	 * access to be common.
	 */
//  printf("stp->segtab == %p\n", stp->seg_tab);
	pt_entry_t **ptp = &stp->seg_tab[kern_phy_to_virt(IOM_RAM_BEGIN) >> SEGSHIFT];
//  printf("ptp == %p\n", ptp);
	pt_entry_t *ptep = (void *)kv_segtabs;
//  printf("ptep == %p\n", ptep);
	memset(ptep, 0, NBPG * kv_nsegtabs);
//  printf("ptp == %p\n", ptp);
	for (size_t i = 0; i < kv_nsegtabs; i++, ptep += NPTEPG) {
//    printf("*0x%08x = 0x%08x\n", (unsigned int)ptp, (unsigned int)ptep);
		*ptp++ = ptep;
	}

  /* Map the kernel text and data to kernel virtual address space 
   * by filling in the page table pages with page table elements
   */

  // No need to fill page table for the kernel text and data in theory
  // since the kernel is loaded in the ram window 

  pt_entry_t *pte = (pt_entry_t *)kv_segtabs;
  for (size_t i = kern_phy_to_virt(IOM_RAM_BEGIN); i < kern_phy_to_virt(kernend) ; i += NBPG)
  {
//    printf("pte == %p ; pte[%d](%p) = 0x%08x\n", pte, (i & L2_MASK) >> PGSHIFT, &pte[(i & L2_MASK) >> PGSHIFT], (unsigned int)(kern_virt_to_phy(i) | PTE_xX | PTE_xW));
    pte[(i & L2_MASK) >> PGSHIFT] = kern_virt_to_phy(i) | PTE_xX | PTE_xW;
  }

  printf("uvm_page_physload(%#lx, %#lx, %#lx, %#lx, %d);\n", atop(kernend), atop(avail[0].start + avail[0].size) - 2, atop(kernend), atop(avail[0].start + avail[0].size) - 2, VM_FREELIST_DEFAULT);
	uvm_page_physload(
		atop(kernend), atop(avail[0].start + avail[0].size) - 2,
		atop(kernend), atop(avail[0].start + avail[0].size) - 2,
		VM_FREELIST_DEFAULT);
//  printf("vm_nphysseg == %d\n", vm_nphysseg);

  pool_init(&pmap_pmap_pool, PMAP_SIZE, 0, 0, 0, "pmappl",
    &pool_allocator_nointr, IPL_NONE);
  pool_init(&pmap_pv_pool, sizeof(struct pv_entry), 0, 0, 0, "pvpl",
   &pmap_pv_page_allocator, IPL_NONE);
  tlb_set_asid(0);

  volatile unsigned long int *dtlb_miss_handler = (unsigned long int *)_dtlb_miss_handler;
  volatile unsigned long int *itlb_miss_handler = (unsigned long int *)_itlb_miss_handler;
  unsigned long int *real_tlb_miss_handler = (unsigned long int *)_real_tlb_miss_handler;
  unsigned long int miss_handler_offset = real_tlb_miss_handler - dtlb_miss_handler;
  unsigned long int bi_opcode = 0xE0000000;
  unsigned long int branch_to_real_tlb_handler_opcode = bi_opcode | miss_handler_offset;

  asm volatile("nop");
  int ok = 0;
//  printf("stp->seg_tab == %p\n", stp->seg_tab);
//  printf("0x%08x >> SEGSHIFT == 0x%08x\n", (unsigned int)VM_MIN_KERNEL_ADDRESS, (unsigned int)(VM_MIN_KERNEL_ADDRESS >> SEGSHIFT));
  for (size_t i = 0xc0000000 + IOM_RAM_SIZE ; i+NBPG < VM_MAX_KERNEL_ADDRESS ; i+= NBPG)
  {
    pt_entry_t *uart_pte = stp->seg_tab[i >> SEGSHIFT];
    if (uart_pte == NULL)
    {
//      printf("L1 ptp page not allocated %d\n", i >> SEGSHIFT);
      vaddr_t new_ptp = uvm_pageboot_alloc(NBPG);
      stp->seg_tab[i >> SEGSHIFT] = (pt_entry_t *)new_ptp;
      i -= NBPG;
      continue;
    }
    if (uart_pte[(i & L2_MASK) >> PGSHIFT] != 0)
    {
//      printf("uart_pte = %p, addr 0x%08x is already mapped. uart_pte[%d] == 0x%08x\n", uart_pte, (unsigned int)i, (i & L2_MASK) >> PGSHIFT, (unsigned int)uart_pte[(i & L2_MASK) >> PGSHIFT]);
      continue;
    } else  {
//      printf("uart_pte[%d] = 0xe0000000\n", (i & L2_MASK) >> PGSHIFT);
//      printf("uart_base_vaddr = 0x%08x\n", i);
      uart_pte[(i & L2_MASK) >> PGSHIFT] = 0xe0000001;
      uart_base_vaddr = i;
      ok = 1;
      break;
    }
  }

  if (!ok)
    panic("Could not map uart to kernel virtual memory!\n");

//  printf("> Patching DTLB and ITL miss handlers...\n");
  *dtlb_miss_handler = branch_to_real_tlb_handler_opcode;
  miss_handler_offset = real_tlb_miss_handler - itlb_miss_handler;
  branch_to_real_tlb_handler_opcode = bi_opcode | miss_handler_offset;
  *itlb_miss_handler = branch_to_real_tlb_handler_opcode;

  lm32_icache_invalidate();
//  printf("We mapped paddr 0xe0000000 (uart_base_paddr) to vaddr 0x%08x", (unsigned int) uart_base_vaddr);

}
void tlbflush(void)
{
//	printf("tlbflush()\n");
	/* flush DTLB */
	asm volatile("xor r11, r11, r11\n\t"
		     "ori r11, r11, 0x3\n\t"
		     "wcsr tlbvaddr, r11" ::: "r11");	

	/* flush ITLB */
	asm volatile("xor r11, r11, r11\n\t"
		     "ori r11, r11, 0x2\n\t"
		     "wcsr tlbvaddr, r11" ::: "r11");	
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
	printf("pmap_load()\n");
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
	KASSERT(ci->ci_current_ipl < IPL_HIGH); 

	KASSERT(l != NULL);
	pmap = vm_map_pmap(&l->l_proc->p_vmspace->vm_map);
	KASSERT(pmap != pmap_kernel());
	oldpmap = ci->ci_curpm;
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
	ci->ci_curpm = pmap;


//	u_int gen = uvm_emap_gen_return();
	cpu_load_pmap(pmap, oldpmap);
//	uvm_emap_update(gen);

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

paddr_t pmap_phys_address(paddr_t cookie)
{
	return (lm32_ptob(cookie));
}

void cpu_load_pmap(struct pmap *new, struct pmap *old)
{
	printf("cpu_load_pmap()\n");
}

void
pmap_md_page_syncicache(struct vm_page *pg, __cpuset_t onproc)
{
	/*
	 * If onproc is empty, we could do a
	 * pmap_page_protect(pg, VM_PROT_NONE) and remove all
	 * mappings of the page and clear its execness.  Then
	 * the next time page is faulted, it will get icache
	 * synched.  But this is easier. :)
	 */
  /* LM32 does not have a way to invalidate only
  *  one cache entry.
  *  moreover data cache is write through so
  *  we only invalidate all instruction cache */
	lm32_icache_invalidate();
}

vaddr_t
pmap_md_map_poolpage(paddr_t pa, vsize_t size)
{
	const vaddr_t sva = (vaddr_t) pa - 0x40000000 + 0xc0000000;
	return sva;
}

void
pmap_md_unmap_poolpage(vaddr_t va, vsize_t size)
{
#ifdef PMAP_MINIMALTLB
	struct pmap * const pm = pmap_kernel();
	const vaddr_t eva = va + size;
	pmap_kvptefill(va, eva, 0);
	for (;va < eva; va += NBPG) {
		pmap_tlb_invalidate_addr(pm, va);
	}
	pmap_update(pm);
#endif
}

void
pmap_md_init(void)
{

	/* nothing for now */
}

bool
pmap_md_direct_mapped_vaddr_p(vaddr_t va)
{
	return va < VM_MIN_KERNEL_ADDRESS || VM_MAX_KERNEL_ADDRESS <= va;
}

paddr_t
pmap_md_direct_mapped_vaddr_to_paddr(vaddr_t va)
{
	return (paddr_t) va;
}

bool
pmap_md_io_vaddr_p(vaddr_t va)
{
	return va >= pmap_limits.avail_end
	    && !(VM_MIN_KERNEL_ADDRESS <= va && va < VM_MAX_KERNEL_ADDRESS);
}

bool
pmap_md_tlb_check_entry(void *ctx, vaddr_t va, tlb_asid_t asid, pt_entry_t pte)
{
	pmap_t pm = ctx;

	const pt_entry_t * const ptep = pmap_pte_lookup(pm, va);
	KASSERT(ptep != NULL);
	pt_entry_t xpte = *ptep;
	xpte &= ~((xpte & (PTE_UNSYNCED|PTE_UNMODIFIED)) << 1);
	xpte ^= xpte & (PTE_UNSYNCED|PTE_UNMODIFIED|PTE_WIRED);

	KASSERTMSG(pte == xpte,
	    "pm=%p va=%#"PRIxVADDR" asid=%u: TLB pte (%#x) != real pte (%#x/%#x)",
	    pm, va, asid, pte, xpte, *ptep);

	return true;
}

#define PSW_ASID_MASK (0x1F000)

void
tlb_set_asid(tlb_asid_t asid)
{
  unsigned int psw;

  asm volatile("rcsr %0, PSW" : "=r"(psw) :: );

  psw |= (asid << 12) & PSW_ASID_MASK;

  asm volatile("wcsr PSW, %0" :: "r"(psw) : );
}
