/*	$NetBSD: $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)asm.h	5.5 (Berkeley) 5/7/91
 */

#ifndef _LM32_ASM_H_
#define	_LM32_ASM_H_

/*
 * The old NetBSD ELF toolchain used underscores.  The new
 * NetBSD ELF toolchain does not.  The C pre-processor
 * defines __NO_LEADING_UNDERSCORES__ for the new ELF toolchain.
 */

#if defined(__ELF__) && defined(__NO_LEADING_UNDERSCORES__)
# define _C_LABEL(x)	x
#else
#ifdef __STDC__
# define _C_LABEL(x)	_ ## x
#else
# define _C_LABEL(x)	_/**/x
#endif
#endif
#define	_ASM_LABEL(x)	x

#ifdef __STDC__
# define __CONCAT(x,y)	x ## y
# define __STRING(x)	#x
#else
# define __CONCAT(x,y)	x/**/y
# define __STRING(x)	"x"
#endif

/* let kernels and others override entrypoint alignment */
#ifndef _ALIGN_TEXT
# define _ALIGN_TEXT .align 2
#endif

#ifdef __ELF__
#define	_ENTRY(x)							\
	.text								;\
	_ALIGN_TEXT							;\
	.globl x							;\
	.type x,@function						;\
	x:
#else /* !__ELF__ */
#define	_ENTRY(x)							\
	.text								;\
	_ALIGN_TEXT							;\
	.globl x							;\
	x:
#endif /* !__ELF__ */

#ifdef GPROF
#define	_PROF_PROLOGUE
#else  /* !GPROF */
#define	_PROF_PROLOGUE
#endif /* !GPROF */

#define	ENTRY(y)	_ENTRY(_C_LABEL(y)) _PROF_PROLOGUE
#define	NENTRY(y)	_ENTRY(_C_LABEL(y))
#define	ASENTRY(y)	_ENTRY(_ASM_LABEL(y)) _PROF_PROLOGUE

#define GET_CPUVAR(reg,off) \
	mvhi	r25, hi(_C_LABEL(cpu_info_store)) ; \
	ori	r25, r25, lo(_C_LABEL(cpu_info_store)) ; \
	lw	reg, (r25+__CONCAT(CPU_INFO_,off))

#define GET_CPUVAR_MMUOFF(reg,off) \
	mvhi	r25, hi(_C_LABEL(cpu_info_store)) ; \
	ori	r25, r25, lo(_C_LABEL(cpu_info_store)) ; \
	mvhi	r24, 0xc000 ; \
	sub	r25, r25, r24 ; \
	mvhi	r24, 0x4000 ; \
	add	r25, r24, r25 ; \
	lw	reg, (r25+__CONCAT(CPU_INFO_,off))

#define DEREF_MMUOFF(dst_reg,src_reg_addr,off) \
	mvhi	r25, 0xc000 ; \
	sub	src_reg_addr, src_reg_addr, r25 ; \
	mvhi	r25, 0x4000 ; \
	add	src_reg_addr, src_reg_addr, r25 ; \
	lw	dst_reg, (src_reg_addr+off)

#define SET_CPUVAR(off,reg) \
	mvhi	r25, hi(_C_LABEL(cpu_info_store)) ; \
	ori	r25, r25, lo(_C_LABEL(cpu_info_store)) ; \
	sw	(r25+__CONCAT(CPU_INFO_,off)), reg

#define SET_ENTRY_SIZE(y) \
	.size	_C_LABEL(y), . - _C_LABEL(y)

#define SET_ASENTRY_SIZE(y) \
	.size	_ASM_LABEL(y), . - _ASM_LABEL(y)

#ifdef __ELF__
#define	ALTENTRY(name)				 \
	.globl _C_LABEL(name)			;\
	.type _C_LABEL(name),@function		;\
	_C_LABEL(name):
#else
#define	ALTENTRY(name)				 \
	.globl _C_LABEL(name)			;\
	_C_LABEL(name):
#endif


/*
 * Hide the gory details of PIC calls vs. normal calls.  Use as in the
 * following example:
 *
 *	sts.l	pr, @-sp
 *	PIC_PROLOGUE(.L_got, r0)	! saves old r12 on stack
 *	...
 *	mov.l	.L_function_1, r0
 * 1:	CALL	r0			! each call site needs a label
 *	 nop
 *      ...
 *	mov.l	.L_function_2, r0
 * 2:	CALL	r0
 *	 nop
 *	...
 *	PIC_EPILOGUE			! restores r12 from stack
 *	lds.l	@sp+, pr		!  so call in right order 
 *	rts
 *	 nop
 *
 *	.align 2
 * .L_got:
 *	PIC_GOT_DATUM
 * .L_function_1:			! if you call the same function twice
 *	CALL_DATUM(function, 1b)	!  provide call datum for each call
 * .L_function_2:
 * 	CALL_DATUM(function, 2b)
 */

#ifdef PIC

#define	PIC_PLT(x)	x@PLT
#define	PIC_GOT(x)	x@GOT
#define	PIC_GOTOFF(x)	x@GOTOFF

#define	PIC_PROLOGUE(got)			\
;FIXME: This needs to be replaced by lm32 asm   \
;        	mov.l	r12, @-sp;		\
;		PIC_PROLOGUE_NOSAVE(got)

/*
 * Functions that do non local jumps don't need to preserve r12,
 * so we can shave off two instructions to save/restore it.
 */
#define	PIC_PROLOGUE_NOSAVE(got)		\
;FIXME: This needs to be replaced by lm32 asm   \
;        	mov.l	got, r12;		\
;        	mova	got, r0;		\
;        	add	r0, r12

#define	PIC_EPILOGUE				\
;FIXME: This needs to be replaced by lm32 asm   \
;		mov.l	@sp+, r12

#define PIC_EPILOGUE_SLOT 			\
		PIC_EPILOGUE

#define PIC_GOT_DATUM \
;FIXME: This needs to be replaced by lm32 asm   \
;		.long	_GLOBAL_OFFSET_TABLE_

#define CALL_DATUM(function, lpcs) \
;FIXME: This needs to be replaced by lm32 asm   \
;		.long	PIC_PLT(function) - ((lpcs) + 4 - (.))

/*
 * This will result in text relocations in the shared library,
 * unless the function is local or has hidden or protected visibility.
 * Does not require PIC prologue.
 */
#define CALL_DATUM_LOCAL(function, lpcs) \
;FIXME: This needs to be replaced by lm32 asm   \
;		.long	function - ((lpcs) + 4)

#else  /* !PIC */

#define	PIC_PROLOGUE(label)
#define	PIC_PROLOGUE_NOSAVE(label)
#define	PIC_EPILOGUE
#define	PIC_EPILOGUE_SLOT	nop
#define PIC_GOT_DATUM

#define CALL_DATUM(function, lpcs) \
;FIXME: This needs to be replaced by lm32 asm   \
;		.long	function

#define CALL_DATUM_LOCAL(function, lpcs) \
;FIXME: This needs to be replaced by lm32 asm   \
;		.long	function

#endif /* !PIC */


#define	ASMSTR		.asciz

#ifdef __ELF__
#define RCSID(x)	.pushsection ".ident"; .asciz x; .popsection
#else
#define	RCSID(x)	.text; .asciz x
#endif

#ifdef NO_KERNEL_RCSIDS
#define	__KERNEL_RCSID(_n, _s)	/* nothing */
#else
#define	__KERNEL_RCSID(_n, _s)	RCSID(_s)
#endif

#ifdef __ELF__
#define	WEAK_ALIAS(alias,sym)						\
	.weak _C_LABEL(alias);						\
	_C_LABEL(alias) = _C_LABEL(sym)
#endif

/*
 * STRONG_ALIAS: create a strong alias.
 */
#define STRONG_ALIAS(alias,sym)						\
	.globl _C_LABEL(alias);						\
	_C_LABEL(alias) = _C_LABEL(sym)

#ifdef __STDC__
#define	WARN_REFERENCES(sym,msg)					\
	.pushsection .gnu.warning. ## sym;				\
	.ascii msg;							\
	.popsection
#else
#define	WARN_REFERENCES(sym,msg)					\
	.pushsection .gnu.warning./**/sym;				\
	.ascii msg;							\
	.popsection
#endif /* __STDC__ */

#endif /* !_LM32_ASM_H_ */
