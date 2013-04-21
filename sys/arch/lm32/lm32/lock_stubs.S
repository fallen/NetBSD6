/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/asm.h>

/*
 * int _atomic_cas_32(volatile uint32_t *val, uint32_t old, uint32_t new);
 *
 * The lm32 softcore does not have a cas instruction, so we implement this as
 * a restartable atomic sequence.  For an example of how this is used,
 * see sun68k/sun68k/isr.c or vax/vax/trap.c
 */
ENTRY(_atomic_cas_32)

	.globl _C_LABEL(_atomic_cas_ras_start)
_C_LABEL(_atomic_cas_ras_start):
	lw	r4, (r1+0) /* load *val into r4 */
	bne	r4, r2, 1f /* compare r4 (*val) and old (r2) */
	sw	(r1+0), r3
	.globl	_C_LABEL(_atomic_cas_ras_end)
_C_LABEL(_atomic_cas_ras_end):

1:
	mv	r1, r4	/* return (*val) */
	ret

STRONG_ALIAS(atomic_cas_ptr,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_ptr,_atomic_cas_32)
STRONG_ALIAS(atomic_cas_uint,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_uint,_atomic_cas_32)
STRONG_ALIAS(atomic_cas_ulong,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_ulong,_atomic_cas_32)
STRONG_ALIAS(atomic_cas_32,_atomic_cas_32)

STRONG_ALIAS(atomic_cas_ptr_ni,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_ptr_ni,_atomic_cas_32)
STRONG_ALIAS(atomic_cas_uint_ni,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_uint_ni,_atomic_cas_32)
STRONG_ALIAS(atomic_cas_ulong_ni,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_ulong_ni,_atomic_cas_32)
STRONG_ALIAS(atomic_cas_32_ni,_atomic_cas_32)
STRONG_ALIAS(_atomic_cas_32_ni,_atomic_cas_32)
