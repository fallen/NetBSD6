/*	$NetBSD: hn_dref_linux.h,v 1.1.1.1 2008/09/19 20:07:17 christos Exp $	*/

/* $srcdir/conf/hn_dref/hn_dref_linux.h */
#define NFS_HN_DREF(dst, src) xstrlcpy((dst), (src), MAXHOSTNAMELEN)
