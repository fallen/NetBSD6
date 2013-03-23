/*	$NetBSD: cdefs.h,v 1.9 2012/01/20 14:08:06 joerg Exp $	*/

#ifndef	_LM32_CDEFS_H_
#define	_LM32_CDEFS_H_

#if defined(_STANDALONE)
#define	__compactcall	__attribute__((__regparm__(3)))
#endif

#define __ALIGNBYTES	(sizeof(int) - 1)

#endif /* !_LM32_CDEFS_H_ */
