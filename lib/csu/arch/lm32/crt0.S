#include <lm32/asm.h>

.hidden __start

STRONG_ALIAS(_start, __start)
ENTRY(__start)
	mvhi r1, hi(_C_LABEL(___start))
	ori  r1, r1, lo(_C_LABEL(___start))
	call r1
