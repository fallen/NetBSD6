/*	$NetBSD: ntp_syscall.h,v 1.1.1.1 2009/12/13 16:54:54 kardel Exp $	*/

/*
 * ntp_syscall.h - various ways to perform the ntp_adjtime() and ntp_gettime()
 * 		   system calls.
 */

#ifndef NTP_SYSCALL_H
#define NTP_SYSCALL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TIMEX_H
# include <sys/timex.h>
#endif

#ifndef NTP_SYSCALLS_LIBC
#ifdef NTP_SYSCALLS_STD
# define ntp_adjtime(t)		syscall(SYS_ntp_adjtime, (t))
# define ntp_gettime(t)		syscall(SYS_ntp_gettime, (t))
#else /* !NTP_SYSCALLS_STD */
# ifdef HAVE___ADJTIMEX
extern	int	__adjtimex	(struct timex *);

#  define ntp_adjtime(t)	__adjtimex((t))

#ifndef HAVE_STRUCT_NTPTIMEVAL
struct ntptimeval
{
  struct timeval time;  /* current time (ro) */
  long int maxerror;    /* maximum error (us) (ro) */
  long int esterror;    /* estimated error (us) (ro) */
};
#endif

static inline int
ntp_gettime(
	struct ntptimeval *ntv
	)
{
	struct timex tntx;
	int result;

	tntx.modes = 0;
	result = __adjtimex (&tntx);
	ntv->time = tntx.time;
	ntv->maxerror = tntx.maxerror;
	ntv->esterror = tntx.esterror;
#ifdef NTP_API
# if NTP_API > 3
	ntv->tai = tntx.tai;
# endif
#endif
	return(result);
}
# else /* !HAVE__ADJTIMEX */
#  ifdef HAVE___NTP_GETTIME
#   define ntp_gettime(t)	__ntp_gettime((t))
#  endif
# endif /* !HAVE_ADJTIMEX */
#endif /* !NTP_SYSCALLS_STD */
#endif /* !NTP_SYSCALLS_LIBC */

#endif /* NTP_SYSCALL_H */
