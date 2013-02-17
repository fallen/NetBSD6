/*	$NetBSD: sdread.c,v 1.6 2010/03/25 15:00:20 pooka Exp $	*/

/*
 * Copyright (c) 2009 Antti Kantee.  All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/mount.h>
#include <sys/dkio.h>

#include <ufs/ufs/ufsmount.h>
#include <msdosfs/msdosfsmount.h>
#include <isofs/cd9660/cd9660_mount.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Proof-of-concept program:
 *
 * Mount rump file system from device driver stack included in the
 * rump kernel.  Optionally copy a file out of the mounted file system.
 */

/* recent -current, appease 5.0 etc. userland */
#ifndef DIOCTUR
#define DIOCTUR _IOR('d', 128, int)
#endif

static void
waitcd(void)
{
	int fd, val = 0, rounds = 0;

	fd = rump_sys_open("/dev/rcd0d", O_RDWR);
	if (fd == -1)
		return;

	do {
		if (rounds > 0) {
			if (rounds == 1) {
				printf("Waiting for CD device to settle ");
			} else {
				printf(".");
			}
			fflush(stdout);
			sleep(1);
		} 
		if (rump_sys_ioctl(fd, DIOCTUR, &val) == -1)
			err(1, "DIOCTUR");
		rounds++;
	} while (val == 0 || rounds >= 30);

	if (!val)
		printf(" giving up\n");
	else
		printf(" done!\n");

	rump_sys_close(fd);
}

int
main(int argc, char *argv[])
{
	char buf[2048];
	struct msdosfs_args args;
	struct ufs_args uargs;
	struct iso_args iargs;
	struct dirent *dp;
	const char *msg = NULL;
	int fd, n, fd_h, sverrno;
	int probeonly = 0;

	if (argc > 1) {
		if (argc == 2 && strcmp(argv[1], "probe") == 0) {
			probeonly = 1;
		} else if (argc != 3) {
			fprintf(stderr, "usage: a.out [src hostdest]\n");
			exit(1);
		}
	}

	memset(&args, 0, sizeof(args));
	args.fspec = strdup("/dev/sd0e");
	args.version = MSDOSFSMNT_VERSION;

	memset(&uargs, 0, sizeof(uargs));
	uargs.fspec = strdup("/dev/sd0e");

	memset(&iargs, 0, sizeof(iargs));
	iargs.fspec = strdup("/dev/cd0a");

	if (probeonly)
		rump_boot_sethowto(RUMP_AB_VERBOSE);
	rump_init();
	if (probeonly) {
		pause();
		exit(0);
	}

	if (rump_sys_mkdir("/mp", 0777) == -1)
		err(1, "mkdir");
	if (rump_sys_mount(MOUNT_MSDOS, "/mp", MNT_RDONLY,
	    &args, sizeof(args)) == -1) {
		if (rump_sys_mount(MOUNT_FFS, "/mp", MNT_RDONLY,
		    &uargs, sizeof(uargs)) == -1) {
			/*
			 * Wait for CD media to settle.  In the end,
			 * just try to do it anyway and see if we fail.
			 */
			waitcd();
			if (rump_sys_mount(MOUNT_CD9660, "/mp", MNT_RDONLY,
			    &iargs, sizeof(iargs)) == -1) {
				err(1, "mount");
			}
		}
	}

	fd = rump_sys_open("/mp", O_RDONLY, 0);
	if (fd == -1) {
		msg = "open dir";
		goto out;
	}

	while ((n = rump_sys_getdents(fd, buf, sizeof(buf))) > 0) {
		for (dp = (struct dirent *)buf;
		    (char *)dp - buf < n;
		    dp = _DIRENT_NEXT(dp)) {
			printf("%" PRIu64 ": %s\n", dp->d_fileno, dp->d_name);
		}
	}
	rump_sys_close(fd);
	if (argc == 1)
		goto out;

	rump_sys_chdir("/mp");
	fd = rump_sys_open(argv[1], O_RDONLY, 0);
	if (fd == -1) {
		msg = "open fs file";
		goto out;
	}

	fd_h = open(argv[2], O_RDWR | O_CREAT, 0777);
	if (fd_h == -1) {
		msg = "open host file";
		goto out;
	}

	while ((n = rump_sys_read(fd, buf, sizeof(buf))) == sizeof(buf)) {
		if (write(fd_h, buf, sizeof(buf)) != sizeof(buf)) {
			msg = "write host file";
			goto out;
		}
	}
	if (n == -1) {
		msg = "read fs file";
		goto out;
	}

	if (n > 0) {
		if (write(fd_h, buf, n) == -1)
			msg = "write tail";
	}

 out:
	sverrno = errno;
	rump_sys_chdir("/");
	rump_sys_close(fd);
	close(fd_h);
	if (rump_sys_unmount("/mp", 0) == -1)
		err(1, "unmount");

	if (msg) {
		errno = sverrno;
		err(1, "%s", msg);
	}

	return 0;
}
