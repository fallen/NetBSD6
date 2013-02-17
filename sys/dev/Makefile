#	$NetBSD: Makefile,v 1.35 2012/06/20 21:38:26 sjg Exp $

SUBDIR=	apm ata bluetooth dec dm dmover dtv filemon hpc \
	i2c i2o ic ieee1394 ir isa \
	microcode ofw pci pckbport pcmcia pud putter raidframe sbus scsipi \
	sun tc usb vme wscons

.include <bsd.own.mk>

.if ${MKISCSI} != "no"
SUBDIR+= iscsi
.endif

INCSDIR= /usr/include/dev

# Only install includes which are used by userland
INCS=	biovar.h ccdvar.h cgdvar.h fssvar.h keylock.h kttcpio.h lockstat.h \
	md.h vndvar.h

.include <bsd.kinc.mk>
