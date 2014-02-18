#ifndef _MACHINE_CSR_H_
#define _MACHINE_CSR_H_

void milkymist_write_csr(int addr, int value);
int milkymist_read_csr(int addr);

#define CSR_TIMER0_BASE	    (0xe0001000)
#define CSR_TIMER0_CONTROL	(0x10)
#define CSR_TIMER0_COMPARE	(0x14)
#define CSR_TIMER0_COUNTER	(0x18)

#define CSR_TIMER1_BASE	  (0xe0001000)
#define CSR_TIMER1_CONTROL	(0x20)
#define CSR_TIMER1_COMPARE	(0x24)
#define CSR_TIMER1_COUNTER	(0x28)

#define TIMER_ENABLE		(0x01)
#define TIMER_AUTORESTART	(0x02)

#endif
