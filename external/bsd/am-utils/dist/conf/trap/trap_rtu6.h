/*	$NetBSD: trap_rtu6.h,v 1.1.1.1 2008/09/19 20:07:19 christos Exp $	*/

/* $srcdir/conf/trap/trap_rtu6.h */
#define	MOUNT_TRAP(type, mnt, flags, mnt_data) 	vmount(type, mnt->mnt_dir, flags, mnt_data)
