/*	$NetBSD: read_line.c,v 1.9 2009/10/21 00:01:57 snj Exp $	*/

/*
 * Copyright (c) 1994 Mats O Jansson <moj@stacken.kth.se>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(lint) && defined(__RCSID)
__RCSID("$NetBSD: read_line.c,v 1.9 2009/10/21 00:01:57 snj Exp $");
#endif

#include <sys/param.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "supcdefs.h"
#include "supextern.h"

/* read_line():
 *	Read a line from a file, parsing continuations ending in \
 *	and eliminating trailing newlines.
 *	Returns a pointer to an internal buffer that is reused upon
 *	next invocation.
 *
 * NOTE: if HAS_FPARSELN is not defined, delim and flags are currently unused.
 */
char *
read_line(FILE * fp, size_t * size, size_t * lineno, const char *delim,
	  int flags)
{
	static char *buf;
#ifdef HAS_FPARSELN

	if (buf != NULL)
		free(buf);
	return (buf = fparseln(fp, size, lineno, delim, flags));
#else
	char *n;
#ifndef HAS_FGETLN
	char sbuf[1024];
#endif
	static int buflen;

	size_t s, len;
	char *ptr;
	int cnt;

	len = 0;
	cnt = 1;
	while (cnt) {
		if (lineno != NULL)
			(*lineno)++;
#ifdef HAS_FGETLN
		if ((ptr = fgetln(fp, &s)) == NULL) {
			if (size != NULL)
				*size = len;
			if (len == 0)
				return NULL;
			else
				return buf;
		}
#else
		if ((ptr = fgets(sbuf, sizeof(sbuf) - 1, fp)) == NULL) {
			if (len == 0)
				return NULL;
			else
				return buf;
		} else {
			char *l;
			if ((l = strchr(sbuf, '\n')) == NULL) {
				if (sbuf[sizeof(sbuf) - 3] != '\\') {
					s = sizeof(sbuf);
					sbuf[sizeof(sbuf) - 2] = '\\';
					sbuf[sizeof(sbuf) - 1] = '\0';
				} else
					s = sizeof(sbuf) - 1;
			} else {
				s = l - sbuf;
			}
		}
#endif
		if (ptr[s - 1] == '\n')	/* the newline may be missing at EOF */
			s--;	/* forget newline */
		if (!s)
			cnt = 0;
		else {
			if ((cnt = (ptr[s - 1] == '\\')) != 0)
				s--;	/* forget \\ */
		}

		if (len + s + 1 > buflen) {
			n = realloc(buf, len + s + 1);
			if (n == NULL)
				err(1, "can't realloc");
			buf = n;
			buflen = len + s + 1;
		}
		memcpy(buf + len, ptr, s);
		len += s;
		buf[len] = '\0';
	}
	if (size != NULL)
		*size = len;
	return buf;
#endif				/* HAS_FPARSELN */
}
