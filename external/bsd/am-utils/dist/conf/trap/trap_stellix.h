/*	$NetBSD: trap_stellix.h,v 1.1.1.1 2008/09/19 20:07:19 christos Exp $	*/

/* $srcdir/conf/trap/trap_stellix.h */
extern int mount_stellix(char *fsname, char *dir, int flags, int type, void *data);
#define	MOUNT_TRAP(type, mnt, flags, mnt_data) 	mount_stellix(mnt->mnt_fsname, mnt->mnt_dir, flags, type, mnt_data)
