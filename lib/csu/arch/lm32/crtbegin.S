#include <lm32/asm.h>

	.section	.ctors, "aw", @progbits
	.p2align 2
__CTOR_LIST__:
	.long -1

	.section	.dtors, "aw", @progbits
	.p2align 2
__DTOR_LIST__:
	.long -1

	.section	.eh_frame, "a", @progbits
	.p2align 2
__EH_FRAME_LIST__:

	.section	.jcr, "aw", @progbits
	.p2align 2
__JCR_LIST__:

	.section	".got2","aw", @progbits
.LCTOC = . + 32768

	.section	.data.rel, "aw", @progbits
	.p2align 2
	.type	__dso_handle, @object
	.size	__dso_handle, 4
	.globl	__dso_handle
	.hidden	__dso_handle
__dso_handle:
#ifdef SHARED
	.long	__dso_handle
#else
	.long	0
#endif

//	.lcomm	__dwarf_eh_object,8*SZREG,SZREG
//	.lcomm	__initialized,1,1
//	.lcomm	__finished,1,1

	.text
	.weak	__cxa_finalize
	.weak	__deregister_frame_info
	.weak	__register_frame_info
	.weak	_Jv_RegisterClasses


__do_global_dtors_aux:
	nop

__do_global_ctors_aux:
	nop

	.section	.init, "ax", @progbits
	bi	__do_global_ctors_aux
	.section	.fini, "ax", @progbits
	bi	__do_global_dtors_aux
