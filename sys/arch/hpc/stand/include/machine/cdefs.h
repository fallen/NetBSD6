/*	$NetBSD: cdefs.h,v 1.5 2005/12/11 12:17:29 christos Exp $	*/

/* Windows CE architecture */

#define	__BEGIN_MACRO	do {
#define	__END_MACRO	} while (/*CONSTCOND*/0)

#define	NAMESPACE_BEGIN(x)	namespace x {
#define	NAMESPACE_END		}
#define	USING_NAMESPACE(x)	using namespace x;
