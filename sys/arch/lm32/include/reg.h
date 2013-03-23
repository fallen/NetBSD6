/*	$NetBSD: reg.h,v 1.18 2011/02/08 20:20:16 rmind Exp $	*/

/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1982, 1986, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
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
 * from: Utah Hdr: reg.h 1.1 90/07/09
 *
 *	@(#)reg.h	7.2 (Berkeley) 11/3/90
 */

#ifndef _LM32_REG_H_
#define _LM32_REG_H_

/*
 * Register set accessible via /proc/$pid/reg and ptrace()
 */
struct reg {
	int	r_regs[28];	/* R1-R28 , R0 is always 0 */
	int	r_ra;
	int	r_ea;
	int 	r_ba;
};

/*
 * Location of the users' stored
 * registers relative to R1.
 * Usage is u.u_ar0[XX].
 */
#define	R1	(0)
#define	R2	(1)
#define	R3	(2)
#define	R4	(3)
#define	R5	(4)
#define	R6	(5)
#define	R7	(6)
#define	R8	(7)
#define	R9	(8)
#define	R10	(9)
#define	R11	(10)
#define	R12	(11)
#define	R13	(12)
#define	R14	(13)
#define	R15	(14)
#define	R16	(15)
#define	R17	(16)
#define	R18	(17)
#define	R19	(18)
#define	R20	(19)
#define	R21	(20)
#define	R22	(21)
#define	R23	(22)
#define	R24	(23)
#define	R25	(24)
#define	R26	(25)
#define	R27	(26)
#define	R28	(27)

#define	RA	(28)
#define	EA	(29)
#define	BA	(30)

#ifdef _KERNEL

struct lwp;
int	process_read_regs(struct lwp *, struct reg *);

#endif

#endif /* !_LM32_REG_H_ */
