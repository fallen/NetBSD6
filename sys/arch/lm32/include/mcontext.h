/*	$NetBSD: $	*/

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Klaus Klein.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LM32_MCONTEXT_H_
#define _LM32_MCONTEXT_H_


/*
 * mcontext extensions to handle signal delivery.
 */
#define _UC_SETSTACK	0x00010000
#define _UC_CLRSTACK	0x00020000
#define _UC_VM		0x00040000
#define	_UC_TLSBASE	0x00080000

/*
 * Layout of mcontext_t according to the ???? Application Binary Interface,
 * LM32 Architecture Processor.
 */  

/*
 * General register state
 */
#define _NGREG		32
typedef	int		__greg_t;
typedef	__greg_t	__gregset_t[_NGREG];

#define _REG_R1		0
#define _REG_R2		1
#define _REG_R3		2
#define _REG_R4		3
#define _REG_R5 	4
#define _REG_R6		5
#define _REG_R7		6
#define _REG_R8		7
#define _REG_R9		8
#define _REG_R10	9
#define _REG_R11	10
#define _REG_R12	11
#define _REG_R13	12
#define _REG_R14	13
#define _REG_R15	14
#define _REG_R16	15
#define _REG_R17	16
#define _REG_R18	17
#define _REG_R19	18
#define _REG_R20	19
#define _REG_R21	20
#define _REG_R22	21
#define _REG_R23	22
#define _REG_R24	23
#define _REG_R25	24
#define _REG_GP		25
#define _REG_FP		26
#define _REG_SP		27
#define _REG_RA		28
#define _REG_EA		29
#define _REG_BA		30
#define _REG_PC		31

typedef struct {
	__gregset_t	__gregs;
} mcontext_t;

#endif	/* !_LM32_MCONTEXT_H_ */
