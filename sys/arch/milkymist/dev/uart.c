#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: uart.c,v 1.11 2010/11/21 18:53:56 ysionneau Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <dev/cons.h>

#include <machine/autoconf.h>
#include <machine/uart.h>

static int uart_match(device_t, cfdata_t, void*);
static void uart_attach(device_t, device_t, void *);

/* Console functions */
static int milkymist_com_cngetc(dev_t);
static void milkymist_com_cnputc(dev_t, int);
static void milkymist_com_cnpollc(dev_t, int);

static struct cnm_state milkymist_com_cnm_state;

CFATTACH_DECL_NEW(uart, 0,
    uart_match, uart_attach, NULL, NULL);

int
uart_match(device_t parent, cfdata_t cf, void *aux)
{
	return 1;
}

void
uart_attach(device_t parent, device_t self, void *aux)
{
	aprint_normal("\n");
}

//TODO: implement com_cngetc
static int milkymist_com_cngetc(dev_t dev)
{
	return 1;
}

//TODO: implement com_cnputc
static void milkymist_com_cnputc(dev_t dev, int c)
{

}

//TODO:implement com_cnpollc
static void milkymist_com_cnpollc(dev_t dev, int on)
{

}

/* console method struct */
struct consdev milkymist_com_cons =
{
    NULL,               /* cn_probe: probe hardware and fill in consdev info */
    NULL,               /* cn_init: turn on as console */
    milkymist_com_cngetc,   /* cn_getc: kernel getchar interface */
    milkymist_com_cnputc,   /* cn_putc: kernel putchar interface */
    milkymist_com_cnpollc,  /* cn_pollc: turn on and off polling */
    NULL,               /* cn_bell: ring bell */
    NULL,               /* cn_halt: stop device */
    NULL,               /* cn_flush: flush output */
    NODEV,              /* major/minor of device */
    CN_NORMAL           /* priority */
};

int milkymist_uart_cnattach(void)
{
	cn_tab = &milkymist_com_cons;
	cn_init_magic(&milkymist_com_cnm_state);

	return 0;
}
