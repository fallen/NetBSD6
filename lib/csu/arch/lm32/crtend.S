#include <machine/asm.h>

	.section	.ctors, "aw", @progbits
	.align 4
	.global		__CTOR_LIST_END__
	.hidden 	__CTOR_LIST_END__
__CTOR_LIST_END__:
	.long 0

	.section	.dtors, "aw", @progbits
	.align 4
	.long 0

	.section	.eh_frame, "a", @progbits
	.align 4
	.long 0

	.section	.jcr, "aw", @progbits
	.align 4
	.long 0
