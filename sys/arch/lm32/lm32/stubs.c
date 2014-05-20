/*	$NetBSD: $	*/

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: booke_stubs.c,v 1.9 2011/06/30 00:52:58 matt Exp $");

#include <sys/param.h>
#include <sys/cpu.h>
#include <lm32/cpuvar.h>

#define	__stub	__section(".stub") __noprofile

u_int tlb_record_asids(u_long *) __stub;

u_int
tlb_record_asids(u_long *bitmap)
{
  //FIXME: TBD
  // We cannot read back our TLB
  // So I guess this function stays like this
  return 0;
}

void tlb_invalidate_asids(tlb_asid_t, tlb_asid_t) __stub;

void
tlb_invalidate_asids(tlb_asid_t asid_lo, tlb_asid_t asid_hi)
{
  //FIXME: TBD
}

void tlb_invalidate_addr(vaddr_t, tlb_asid_t) __stub;

void
tlb_invalidate_addr(vaddr_t va, tlb_asid_t asid)
{
  //FIXME: TBD
}

bool tlb_update_addr(vaddr_t, tlb_asid_t, pt_entry_t, bool) __stub;

bool
tlb_update_addr(vaddr_t va, tlb_asid_t asid, pt_entry_t pte, bool insert_p)
{
  va &= ~((1 << 12)-1); /* remove any in-page offset bit */
  va |= asid << 7;
  // FIXME: fix conditions for DTLB
  if ( pte & PTE_xX )
  {
    asm volatile("wcsr TLBVADDR, %0" :: "r"(va) : );
    asm volatile("wcsr TLBPADDR, %0" :: "r"(pte) : );
  }
  if (/* pte & PROT_READ*/ 1)
  {
    pte |= 1;
    asm volatile("wcsr TLBVADDR, %0" :: "r"(va) : );
    asm volatile("wcsr TLBPADDR, %0" :: "r"(pte) : );
  }
  return true;
}
