#ifndef _MACHINE_CSR_H_
#define _MACHINE_CSR_H_

#ifdef __ASSEMBLER__
#define MMPTR(x) x
#else
#define MMPTR(x) (*((volatile unsigned int *)(x)))
#endif

void milkymist_write_csr(int addr, int value);
int milkymist_read_csr(int addr);

#define CSR_TIMER0_CONTROL	MMPTR(0xe0001010)
#define CSR_TIMER0_COMPARE	MMPTR(0xe0001014)
#define CSR_TIMER0_COUNTER	MMPTR(0xe0001018)

#define CSR_TIMER1_CONTROL	MMPTR(0xe0001020)
#define CSR_TIMER1_COMPARE	MMPTR(0xe0001024)
#define CSR_TIMER1_COUNTER	MMPTR(0xe0001028)

#define TIMER_ENABLE		(0x01)
#define TIMER_AUTORESTART	(0x02)

#endif
