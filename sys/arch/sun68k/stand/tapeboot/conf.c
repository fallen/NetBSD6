/*	$NetBSD: conf.c,v 1.4 2005/12/11 12:19:29 christos Exp $	*/

#include <stand.h>
#include <rawfs.h>
#include <dev_tape.h>

struct fs_ops file_system[] = {
	FS_OPS(rawfs),
};
int nfsys = 1;

struct devsw devsw[] = {
	{ "tape", tape_strategy, tape_open, tape_close, tape_ioctl },
};
int ndevs = 1;

#ifdef DEBUG
int debug;
#endif /* DEBUG */
