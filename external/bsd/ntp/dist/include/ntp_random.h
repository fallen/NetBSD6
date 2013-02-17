/*	$NetBSD: ntp_random.h,v 1.1.1.1 2009/12/13 16:54:52 kardel Exp $	*/


#include <ntp_types.h>

long ntp_random (void);
void ntp_srandom (unsigned long);
void ntp_srandomdev (void);
char * ntp_initstate (unsigned long, 	/* seed for R.N.G. */
			char *,		/* pointer to state array */
			long		/* # bytes of state info */
			);
char * ntp_setstate (char *);	/* pointer to state array */



