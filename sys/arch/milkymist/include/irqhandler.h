/*	$NetBSD: irqhandler.h,v 1.5 2008/04/27 18:58:47 matt Exp $	*/

/*
 * Copyright (c) 1994-1996 Mark Brinicombe.
 * Copyright (c) 1994 Brini.
 * All rights reserved.
 *
 * This code is derived from software written for Brini by Mark Brinicombe
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
 *	This product includes software developed by Mark Brinicombe
 *	for the NetBSD Project.
 * 4. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * IRQ related stuff (defines + structures)
 *
 * Created      : 30/09/94
 */

#ifndef _MACHINE_IRQHANDLER_H_
#define _MACHINE_IRQHANDLER_H_

#include <sys/evcnt.h>

/* Define the IRQ bits */

#define IRQ_USB			0x00
#define IRQ_IR 			0x01
#define IRQ_MIDI 		0x02 
#define IRQ_VIDEOIN 		0x03
#define IRQ_ETHERNET_TX 	0x04
#define IRQ_ETHERNET_RX 	0x05
#define IRQ_TMU 		0x06
#define IRQ_PFPU 		0x07
#define IRQ_AC97_DMA_W		0x08
#define IRQ_AC97_DMA_R		0x09
#define IRQ_AC97_CR_REPLY	0x0a
#define IRQ_AC97_CR_REQUEST	0x0b
#define IRQ_TIMER1		0x0c
#define IRQ_TIMER0		0x0d
#define IRQ_GPIO		0x0e
#define IRQ_UART		0x0f


#define IRQ_INSTRUCT	-1
#define NIRQS		0x10 // 16 IRQs

typedef struct irqhandler {
	int (*ih_func)(void *arg);	/* handler function */
	void *ih_arg;			/* Argument to handler */
	int ih_level;			/* Interrupt level */
	int ih_num;			/* Interrupt number (for accounting) */
	u_int ih_flags;			/* Interrupt flags */
	u_int ih_maskaddr;		/* mask address for expansion cards */
	u_int ih_maskbits;		/* interrupt bit for expansion cards */
	struct irqhandler *ih_next;	/* next handler */
	struct evcnt ih_ev;		/* evcnt structure */
	int (*ih_realfunc)(void *arg);	/* XXX real handler function */
	void *ih_realarg;
} irqhandler_t;

#ifdef _KERNEL
extern u_int irqmasks[NIPL];
extern irqhandler_t *irqhandlers[NIRQS];

void irq_init(void);
int irq_claim(int, irqhandler_t *, const char *group, const char *name);
int irq_release(int, irqhandler_t *);
void *intr_claim(int irq, int level, int (*func)(void *), void *arg,
	const char *group, const char *name);
int intr_release(void *ih);
void irq_setmasks(void);
void disable_irq(int);
void enable_irq(int);
#endif	/* _KERNEL */

#define IRQ_FLAG_ACTIVE 0x00000001	/* This is the active handler in list */

#endif	/* _MACHINE_IRQHANDLER_H_ */

/* End of irqhandler.h */
