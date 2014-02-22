/* 	$NetBSD: intr.h,v 1.9 2010/06/13 02:11:22 tsutsui Exp $	*/

/*
 * Copyright (c) 1997 Mark Brinicombe.
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
 */

#ifndef _MACHINE_INTR_H_
#define _MACHINE_INTR_H_

#include <lm32/intr.h>
#include <machine/cpu.h>
#include <lib/libkern/libkern.h>


#define USB_IRQ 		(0x00008000)
#define IR_IRQ 			(0x00004000)
#define MIDI_IRQ 		(0x00002000)
#define VIDEOIN_IRQ 		(0x00001000)
#define ETHERNET_TX_IRQ 	(0x00000800)
#define ETHERNET_RX_IRQ 	(0x00000400)
#define TMU_IRQ 		(0x00000200)
#define PFPU_IRQ 		(0x00000100)
#define AC97_DMA_W_IRQ 		(0x00000080)
#define AC97_DMA_R_IRQ 		(0x00000040)
#define AC97_CR_REPLY_IRQ 	(0x00000020)
#define AC97_CR_REQUEST_IRQ 	(0x00000010)
#define TIMER1_IRQ 		(0x00000008)
#define TIMER0_IRQ 		(0x00000004)
#define GPIO_IRQ 		(0x00000002)
#define UART_IRQ 		(0x00000001)

#define ALL_IRQ (USB_IRQ | IR_IRQ | MIDI_IRQ | VIDEOIN_IRQ | ETHERNET_TX_IRQ | \
		 ETHERNET_RX_IRQ | TMU_IRQ | PFPU_IRQ | AC97_DMA_W_IRQ | \
		 AC97_DMA_R_IRQ | AC97_CR_REQUEST_IRQ | AC97_CR_REPLY_IRQ | \
		 TIMER0_IRQ | TIMER1_IRQ | GPIO_IRQ | UART_IRQ)

#define NO_IRQ (0x00000000)

static __inline void _mask_irq(int irq_mask)
{
	int old_mask = _get_irq_mask();

	_set_irq_mask( old_mask & ~(irq_mask) );
}

static __inline void _unmask_irq(int irq_mask)
{
	int old_mask = _get_irq_mask();
	_set_irq_mask( old_mask & irq_mask);
}


typedef int ipl_t;
typedef struct {
	ipl_t  _ipl;
} ipl_cookie_t;

ipl_t _splraise(ipl_t level);

static inline ipl_cookie_t
makeiplcookie(ipl_t ipl)
{

	return (ipl_cookie_t){._ipl = ipl};
}

#define LM32_CSR_PSW_IE_SHIFT (0x0)
#define LM32_CSR_PSW_IE (1 << LM32_CSR_PSW_IE_SHIFT)

static inline int
splraiseipl(ipl_cookie_t icookie)
{

	return _splraise(icookie._ipl);
}

//#include <machine/irqhandler.h>
void __isr(unsigned int, unsigned int);

#endif	/* _MACHINE_INTR_H */
