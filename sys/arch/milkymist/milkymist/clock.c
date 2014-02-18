/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/time.h>
#include <sys/device.h>
#include <uvm/uvm_extern.h>

#include <machine/intr.h>
#include <machine/csr.h>

#include <opt_hz.h>     /* for HZ */

#define DEBUG_CLK
#ifdef DEBUG_CLK
#define DPRINTF(fmt...)  printf(fmt)
#else
#define DPRINTF(fmt...)  
#endif

#define SYSCLK_FREQ		(80000000)
#define TIMER0_MAX_VALUE	(SYSCLK_FREQ / HZ)
#define MILKYMIST_USEC_PER_TICK	(1000000 / HZ)

static int clock_match(struct device *, struct cfdata *, void *);
static void clock_attach(struct device *, struct device *, void *);

struct clock_softc {
	struct device       device;
	int                 sc_intr;
  vaddr_t             base_vaddr;
};

static struct clock_softc *clock_sc = NULL;

static int
clock_match(struct device *parent, struct cfdata *match, void *aux)
{
	return 1;
}

static void
clock_attach(struct device *parent, struct device *self, void *aux)
{
	struct clock_softc *sc = (struct clock_softc*) self;
  vaddr_t clock_base_vaddress;

  aprint_normal("\n");
  clock_base_vaddress = uvm_km_alloc(kernel_map, PAGE_SIZE, PAGE_SIZE, UVM_KMF_VAONLY | UVM_KMF_NOWAIT);
  printf("Allocated vaddr %08x for clock SoC memory mapped registers\n", (unsigned int)clock_base_vaddress);
  pmap_kenter_pa(clock_base_vaddress, CSR_TIMER0_BASE, VM_PROT_READ | VM_PROT_WRITE , PMAP_NOCACHE);
  pmap_update(pmap_kernel());

	if (clock_sc == NULL)
		clock_sc = sc;

  clock_sc->base_vaddr = clock_base_vaddress;
}

CFATTACH_DECL_NEW(clock, sizeof(struct clock_softc), clock_match,
	clock_attach, NULL, NULL);

// TODO: what is this supposed to do?
// A lot of port let this empty
void
setstatclockrate(int hertz)
{

}

static __inline void milkymist_timer0_enable_irq(void)
{
	_unmask_irq(TIMER0_IRQ);
}

static __inline void milkymist_timer0_disable_irq(void)
{
	_mask_irq(TIMER0_IRQ);
}

/*
 * clock_intr:
 *
 *Handle the hardclock interrupt.
 */
static int
clock_intr(void *arg)
{

	printf("clock ticked!\n");
	_ack_irq(TIMER0_IRQ);
	return 1;
}

void write_to_csr(vaddr_t base, unsigned int offset, unsigned int value);
void write_to_csr(vaddr_t base, unsigned int offset, unsigned int value)
{
  *(volatile unsigned int *)((unsigned int)base + offset) = value;
}

/*
 * cpu_initclocks:
 *
 *Initialize the clock and get it going.
 */
void
cpu_initclocks(void)
{
	struct clock_softc *sc = clock_sc;

	stathz = profhz = 0;

	/* set up and enable timer 0 as kernel timer, */
	/* using 88 MHz cpu clock source */

	/* register interrupt handler */
	lm32_intrhandler_register(sc->sc_intr, clock_intr);

	/* Enable interrupts from timer 1 */
	milkymist_timer0_enable_irq();

	/* load max value of the timer */
  write_to_csr(sc->base_vaddr, CSR_TIMER0_COMPARE, TIMER0_MAX_VALUE);
	/* start the timer */
  write_to_csr(sc->base_vaddr, CSR_TIMER0_CONTROL, TIMER_ENABLE);
}
