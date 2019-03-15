#include <machine/asm.h>
#include <sys/errno.h>
#include "SYS.h"

#ifdef WEAK_ALIAS
WEAK_ALIAS(clone, __clone)
#endif
	.text

/*
 * int clone(int (*fn)(void *), void *stack, int flags, void *arg);
 */
ENTRY(__clone) \
	ret
