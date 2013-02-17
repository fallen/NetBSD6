/*	$NetBSD: algor_p5064_dma.c,v 1.6 2011/07/08 18:48:55 matt Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
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

/*
 * Platform-specific DMA support for the Algorithmics P-5064.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: algor_p5064_dma.c,v 1.6 2011/07/08 18:48:55 matt Exp $");

#include <sys/param.h>

#define	_MIPS_BUS_DMA_PRIVATE
#include <sys/bus.h>

#include <algor/algor/algor_p5064reg.h>
#include <algor/algor/algor_p5064var.h>

#include <dev/isa/isavar.h>

void
algor_p5064_dma_init(struct p5064_config *acp)
{
	bus_dma_tag_t t;

	/*
	 * Initialize the DMA tag used for PCI DMA.
	 */
	t = &acp->ac_pci_dmat;
	t->_cookie = acp;
	t->_wbase = P5064_DMA_PCI_PCIBASE;
        t->_bounce_alloc_lo = P5064_DMA_PCI_PHYSBASE;
        t->_bounce_alloc_hi = P5064_DMA_PCI_PHYSBASE + P5064_DMA_PCI_SIZE;
	t->_dmamap_ops = mips_bus_dmamap_ops;
	t->_dmamem_ops = mips_bus_dmamem_ops;
	t->_dmatag_ops = mips_bus_dmatag_ops;

	/*
	 * Initialize the DMA tag usd for ISA DMA.
	 */
	t = &acp->ac_isa_dmat;
	t->_cookie = acp;
	t->_wbase = P5064_DMA_ISA_PCIBASE;
        t->_bounce_alloc_lo = P5064_DMA_ISA_PHYSBASE;
        t->_bounce_alloc_hi = P5064_DMA_ISA_PHYSBASE + P5064_DMA_ISA_SIZE;
	t->_dmamap_ops = mips_bus_dmamap_ops;
	t->_dmamem_ops = mips_bus_dmamem_ops;
	t->_dmatag_ops = mips_bus_dmatag_ops;
}
