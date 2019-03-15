#include <machine/asm.h>


#include "sysident.S"

	.section ".init", "ax", @progbits
	.align 16
	.globl _init
_init:

	.section ".fini", "ax", @progbits
	.align 16
	.globl _fini
_fini:
