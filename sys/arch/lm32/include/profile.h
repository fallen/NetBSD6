
#if defined(__ELF__) && defined(__NO_LEADING_UNDERSCORES__)
#define	_MCOUNT_DECL static void _mcount
#else
#define	_MCOUNT_DECL static void mcount
#endif

#define MCOUNT

#ifdef _KERNEL
/*
 * Note that we assume splhigh() and splx() cannot call mcount()
 * recursively.
 */
#define MCOUNT_ENTER    s = splhigh()
#define MCOUNT_EXIT     splx(s)
#endif /* _KERNEL */
