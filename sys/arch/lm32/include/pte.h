/*	$NetBSD: pte.h,v 1.27 2011/02/01 20:09:08 chuck Exp $	*/

/*
 * Copyright (c) 2001 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Frank van der Linden for Wasabi Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the NetBSD Project by
 *      Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1997 Charles D. Cranor and Washington University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LM32_PTE_H_
#define _LM32_PTE_H_


#if !defined(_LOCORE)

#include <sys/types.h>
#include <uvm/uvm_prot.h>
#include <uvm/pmap/vmpagemd.h>
#include <lib/libkern/libkern.h>
/*
 * here we define the data types for PDEs and PTEs
 */
typedef uint32_t pd_entry_t;		/* PDE */
#ifndef __BSD_PT_ENTRY_T
#define __BSD_PT_ENTRY_T	__uint32_t
typedef uint32_t pt_entry_t;		/* PTE */
#endif
#endif

/*
 * now we define various for playing with virtual addresses
 */

#define	PG_FRAME	(0xfffff000)	/* page frame mask */

/* software emulated bits */

#define PG_WIRED		(1 << 11)
#define PG_NOCACHE		(1 << 10)
#define PG_WRITE_COMBINE	(1 << 9)
#define PG_WRITE_BACK		(1 << 8)
#define PG_NOCACHE_OVR		(1 << 7)
#define PG_D			(1 << 6)

/* hardware managed bits */
#define PG_RO			(1 << 1) /* Page read only */

#define PG_PR_MASK		(1 << 1) /* Page protection mask */

#define L1_SHIFT 	(12)
#define L2_SHIFT 	(21)
#define	NBPD_L1		(1ULL << L1_SHIFT) /* # bytes mapped by L1 ent (4K) */
#define	NBPD_L2		(1ULL << L2_SHIFT) /* # bytes mapped by L2 ent (2MB) */

#define L1_MASK 	(0x001ff000)

#define PTE_CACHE_INHIBIT (1 << 2)

#define PTE_M (1 << 3)
#define PTE_G (1 << 4)
#define PTE_xR (1 << 5)
#define PTE_I (1 << 6)
#define PTE_xW (1 << 7)
#define PTE_xX (1 << 8)
#define PTE_UNMODIFIED (1 << 9)
#define PTE_WIRED (1 << 10)
#define PTE_RPN_MASK  (PG_FRAME)
#define PTE_UNSYNCED (1 << 11)

// FIXME: we should not have to redefine this here
// This is defined in sys/uvm/uvm_pmap.h
#define PMAP_NOCACHE    0x00000100  /* [BOTH] */

static inline pt_entry_t
pte_prot_downgrade(pt_entry_t pt_entry, vm_prot_t newprot)
{
	pt_entry &= ~(PTE_xW|PTE_UNMODIFIED);
	if ((newprot & VM_PROT_EXECUTE) == 0)
		pt_entry &= ~(PTE_xX|PTE_UNSYNCED);
	return pt_entry;
}

static inline pt_entry_t
pte_nv_entry(bool kernel)
{
	return 0;
}
static inline paddr_t
pte_to_paddr(pt_entry_t pt_entry)
{
	return (paddr_t)(pt_entry & PTE_RPN_MASK);
}
static inline bool
pte_wired_p(pt_entry_t pt_entry)
{
	return (pt_entry & PTE_WIRED) != 0;
}
static inline bool
pte_cached_p(pt_entry_t pt_entry)
{
	return (pt_entry & PTE_I) == 0;
}

static inline bool
pte_modified_p(pt_entry_t pt_entry)
{
	return (pt_entry & (PTE_UNMODIFIED|PTE_xW)) == PTE_xW;
}

static inline bool
pte_valid_p(pt_entry_t pt_entry)
{
	return pt_entry != 0;
}

static inline bool
pte_exec_p(pt_entry_t pt_entry)
{
	return (pt_entry & PTE_xX) != 0;
}

static inline pt_entry_t
pte_wire_entry(pt_entry_t pt_entry)
{
	return pt_entry | PTE_WIRED;
}

static inline pt_entry_t
pte_prot_nowrite(pt_entry_t pt_entry)
{
	return pt_entry & ~(PTE_xW|PTE_UNMODIFIED);
}

static inline bool
pte_deferred_exec_p(pt_entry_t pt_entry)
{
	//return (pt_entry & (PTE_xX|PTE_UNSYNCED)) == (PTE_xX|PTE_UNSYNCED);
	return (pt_entry & PTE_UNSYNCED) == PTE_UNSYNCED;
}

static inline pt_entry_t
pte_prot_bits(struct vm_page_md *mdpg, vm_prot_t prot)
{
	KASSERT(prot & VM_PROT_READ);
	pt_entry_t pt_entry = PTE_xR;
	if (prot & VM_PROT_EXECUTE) {
#if 0
		pt_entry |= PTE_xX;
		if (mdpg != NULL && !VM_PAGEMD_EXECPAGE_P(mdpg))
			pt_entry |= PTE_UNSYNCED;
#elif 1
		if (mdpg != NULL && !VM_PAGEMD_EXECPAGE_P(mdpg))
			pt_entry |= PTE_UNSYNCED;
		else
			pt_entry |= PTE_xX;
#else
		pt_entry |= PTE_UNSYNCED;
#endif
	}
	if (prot & VM_PROT_WRITE) {
		pt_entry |= PTE_xW;
		if (mdpg != NULL && !VM_PAGEMD_MODIFIED_P(mdpg))
			pt_entry |= PTE_UNMODIFIED;
	}
	return pt_entry;
}

static inline pt_entry_t
pte_ionocached_bits(void)
{
	return PTE_CACHE_INHIBIT;
}

static inline pt_entry_t
pte_iocached_bits(void)
{
	return 0;
}

static inline pt_entry_t
pte_nocached_bits(void)
{
	return PTE_CACHE_INHIBIT;
}

static inline pt_entry_t
pte_cached_bits(void)
{
	return 0; /* by default mapping is cached */
}

static inline pt_entry_t
pte_flag_bits(struct vm_page_md *mdpg, int flags)
{
	if (__predict_false(flags & PMAP_NOCACHE)) {
		if (__predict_true(mdpg != NULL)) {
			return pte_nocached_bits();
		} else {
			return pte_ionocached_bits();
		}
	} else {
		if (__predict_false(mdpg != NULL)) {
			return pte_cached_bits();
		} else {
			return pte_iocached_bits();
		}
	}
}

static inline pt_entry_t
pte_make_enter(paddr_t pa, struct vm_page_md *mdpg, vm_prot_t prot,
	int flags, bool kernel)
{
	pt_entry_t pt_entry = (pt_entry_t) pa & PTE_RPN_MASK;

	pt_entry |= pte_flag_bits(mdpg, flags);
	pt_entry |= pte_prot_bits(mdpg, prot);

	return pt_entry;
}

static inline pt_entry_t
pte_make_kenter_pa(paddr_t pa, struct vm_page_md *mdpg, vm_prot_t prot,
	int flags)
{
	pt_entry_t pt_entry = (pt_entry_t) pa & PTE_RPN_MASK;

	pt_entry |= PTE_WIRED;
	pt_entry |= pte_flag_bits(mdpg, flags);
	pt_entry |= pte_prot_bits(NULL, prot); /* pretend unmanaged */

	return pt_entry;
}
static inline pt_entry_t
pte_unwire_entry(pt_entry_t pt_entry)
{
	return pt_entry & ~PTE_WIRED;
}

#endif /* _LM32_PTE_H_ */
