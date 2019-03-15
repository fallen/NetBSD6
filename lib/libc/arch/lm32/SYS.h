#include <sys/syscall.h>
#include <lm32/asm.h>


#ifdef __STDC__
# define SYSTRAP(x) \
	mvhi r1, hi(SYS_ ## x); \
	ori  r1, r1, lo(SYS_ ## x); \
	scall
#else
# define SYSTRAP(x) \
	mvhi r1, hi(SYS_/**/x); \
	ori  r1, r1, lo(SYS_/**/x); \
	scall
#endif

/*
 * Do a syscall that has an internal name and a weak external alias.
 */
#define	WSYSCALL(weak,strong) 		\
	WEAK_ALIAS(weak,strong); 	\
	PSEUDO(strong, weak);		\
	ret

#define PSEUDO(x,y)			\
	ENTRY(x);			\
	SYSTRAP(y);			\
	ret

#define PSEUDO_NOERROR(x,y)		\
	ENTRY(x);			\
	SYSTRAP(y);			\
	ret

#define RSYSCALL_NOERROR(sym)		\
	PSEUDO_NOERROR(sym, sym)

#define RSYSCALL(sym)			\
	PSEUDO(sym, sym)
