/*     $NetBSD: mnttab.c,v 1.2 2010/02/28 17:36:51 haad Exp $  */

/*-
 * Copyright (c) 2006 Pawel Jakub Dawidek <pjd@FreeBSD.org>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This file implements Solaris compatible getmntany() and hasmntopt()
 * functions.
 */

#include <sys/cdefs.h>
/* __FBSDID("$FreeBSD: src/compat/opensolaris/misc/mnttab.c,v 1.1 2007/04/06 01:08:59 pjd Exp $"); */
__RCSID("$NetBSD: mnttab.c,v 1.2 2010/02/28 17:36:51 haad Exp $");

#include <sys/param.h>
#include <sys/statvfs.h>
#include <sys/mntent.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>

#include <sys/mnttab.h>

char *
mntopt(char **p)
{
	char *cp = *p;
	char *retstr;

	while (*cp && isspace(*cp))
		cp++;

	retstr = cp;
	while (*cp && *cp != ',')
		cp++;

	if (*cp) {
		*cp = '\0';
		cp++;
	}

	*p = cp;
	return (retstr);
}

char *
hasmntopt(struct mnttab *mnt, char *opt)
{
	char tmpopts[MNT_LINE_MAX];
	char *f, *opts = tmpopts;

	if (mnt->mnt_mntopts == NULL)
		return (NULL);
	(void) strcpy(opts, mnt->mnt_mntopts);
	f = mntopt(&opts);
	for (; *f; f = mntopt(&opts)) {
		if (strncmp(opt, f, strlen(opt)) == 0)
			return (f - tmpopts + mnt->mnt_mntopts);
	}
	return (NULL);
}

static void
optadd(char *mntopts, size_t size, const char *opt)
{

	if (mntopts[0] != '\0')
		strlcat(mntopts, ",", size);
	strlcat(mntopts, opt, size);
}

int
getmntany(FILE *fd __unused, struct mnttab *mgetp, struct mnttab *mrefp)
{
	static struct statvfs *sfs;
	static char mntopts[MNTMAXSTR];
	struct opt *o;
	static int i, n;
	int flags;

	if (sfs == NULL) {
		n = getmntinfo(&sfs, ST_WAIT);
		if (n == -1)
			return -1;
	}
	for (i = 0; i < n; i++) {
		if (mrefp->mnt_special != NULL &&
		    strcmp(mrefp->mnt_special, sfs[i].f_mntfromname) != 0) {
			continue;
		}
		if (mrefp->mnt_mountp != NULL &&
		    strcmp(mrefp->mnt_mountp, sfs[i].f_mntonname) != 0) {
			continue;
		}
		if (mrefp->mnt_fstype != NULL &&
		    strcmp(mrefp->mnt_fstype, sfs[i].f_fstypename) != 0) {
			continue;
		}
		flags = sfs[i].f_flag;
#define	OPTADD(opt)	optadd(mntopts, sizeof(mntopts), (opt))
		if (flags & MNT_RDONLY)
			OPTADD(MNTOPT_RO);
		else
			OPTADD(MNTOPT_RW);
		if (flags & MNT_NOSUID)
			OPTADD(MNTOPT_NOSUID);
		else
			OPTADD(MNTOPT_SETUID);
		if (flags & MNT_UPDATE)
			OPTADD(MNTOPT_REMOUNT);
		if (flags & MNT_NOATIME)
			OPTADD(MNTOPT_NOATIME);
		else
			OPTADD(MNTOPT_ATIME);
		OPTADD(MNTOPT_NOXATTR);
		if (flags & MNT_NOEXEC)
			OPTADD(MNTOPT_NOEXEC);
		else
			OPTADD(MNTOPT_EXEC);
#undef	OPTADD
		mgetp->mnt_special = sfs[i].f_mntfromname;
		mgetp->mnt_mountp = sfs[i].f_mntonname;
		mgetp->mnt_fstype = sfs[i].f_fstypename;
		mgetp->mnt_mntopts = mntopts;
		return (0);
	}
	sfs = NULL;
	return (-1);
}

int
getstatfs(struct statvfs *stat, const char *path)
{
	int fs_num, i;
	struct statvfs *statvfs;

	fs_num = 0;
	
	if (path == NULL)
		return (-1);

	fs_num = getvfsstat(NULL, 0, ST_WAIT);

	if ((statvfs = malloc(fs_num * sizeof(struct statvfs))) == NULL)
	    return (-1);
	    
	memset(statvfs, 0, fs_num * sizeof(struct statvfs));
	    
	if (getvfsstat(statvfs, fs_num * sizeof(struct statvfs), ST_WAIT) != 0) {
		free(statvfs);
		return (-1);
	}
	
	for( i = 0; i < fs_num; i++) {
		
		if (statvfs[i].f_fstypename != NULL &&
		    strcmp(statvfs[i].f_fstypename, MNTTYPE_ZFS) != 0) {
			continue;
		}
		
		if (statvfs[i].f_mntonname != NULL &&
		    strcmp(statvfs[i].f_mntonname, path) != 0) {
			continue;
		}
		
		memcpy(stat, &statvfs[i], sizeof(struct statvfs));
	}

	free(statvfs);
	return (0);
}

int
getmntent(FILE *file, struct mnttab *mnttab) {


	return 1;
}
