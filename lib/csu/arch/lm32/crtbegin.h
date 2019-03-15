__asm(
	".pushsection	.init, \"ax\", %progbits"
"\n\t"	"bi	__do_global_ctors_aux"
"\n\t"	".popsection");

__asm(
	".pushsection	.fini, \"ax\", %progbits"
"\n\t"	"bi	__do_global_dtors_aux"
"\n\t"	".popsection");
