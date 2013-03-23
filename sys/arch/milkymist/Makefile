#	$NetBSD: Makefile,v 1.8 2011/04/04 19:44:16 dyoung Exp $

# Find where lm32 source files are for inclusion in tags
#.include <../lm32/Makefile.inc>


#.include "../../kern/Make.tags.inc"

#.PATH.S: ${S}/../common/lib/libc/arch/lm32/gen
SRCS+=	divsi3.S udivsi3.S modsi3.S umodsi3.S

SUBDIR=	compile include

.include <bsd.subdir.mk>
