/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/time.h>
#include <sys/device.h>

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

static int milkymist_clock_match(struct device *, struct cfdata *, void *);
static void milkymist_clock_attach(struct device *, struct device *, void *);

struct milkymist_clock_softc {
	struct device       device;
	int                 sc_intr;
};

static struct milkymist_clock_softc *milkymist_clock_sc = NULL;

static int
milkymist_clock_match(struct device *parent, struct cfdata *match, void *aux)
{
	DPRINTF("milkymist_clock_match\n");
	return 1;
}

static void
milkymist_clock_attach(struct device *parent, struct device *self, void *aux)
{
	struct milkymist_clock_softc *sc = (struct milkymist_clock_softc*) self;

	DPRINTF("milkymist_clk_attach\n");

	if (milkymist_clock_sc == NULL)
		milkymist_clock_sc = sc;
}

CFATTACH_DECL_NEW(milkymist_clock, sizeof(struct milkymist_clock_softc), milkymist_clock_match,
	milkymist_clock_attach, NULL, NULL);

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
 * milkymist_clock_intr:
 *
 *Handle the hardclock interrupt.
 */
static int
milkymist_clock_intr(void *arg)
{

	DPRINTF("clock ticked!\n");
	_ack_irq(TIMER0_IRQ);
	return 1;
}

/*
 * cpu_initclocks:
 *
 *Initialize the clock and get it going.
 */
void
cpu_initclocks(void)
{
	struct milkymist_clock_softc *sc = milkymist_clock_sc;

	stathz = profhz = 0;

	/* set up and enable timer 0 as kernel timer, */
	/* using 88 MHz cpu clock source */

	/* register interrupt handler */
	lm32_intrhandler_register(sc->sc_intr, milkymist_clock_intr);

	/* Enable interrupts from timer 1 */
	milkymist_timer0_enable_irq();

	/* load max value of the timer */
	CSR_TIMER0_COMPARE = TIMER0_MAX_VALUE;

	/* start the timer */
	CSR_TIMER0_CONTROL = TIMER_ENABLE;
}
