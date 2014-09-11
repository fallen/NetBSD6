/*	$NetBSD: $	*/

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/kauth.h>
#include <sys/proc.h>
#include <dev/cons.h>

#include <machine/autoconf.h>
#include <machine/uart.h>

struct milkymist_com_softc {
	device_t	sc_dev;
	struct tty	*sc_tty;
};

extern struct cfdriver milkymist_com_cd;

static struct milkymist_com_softc *uart_sc = NULL;

static int uart_match(device_t, cfdata_t, void*);
static void uart_attach(device_t, device_t, void *);

/* Console functions */
static int milkymist_com_cngetc(dev_t);
static void milkymist_com_cnputc(dev_t, int);
static void milkymist_com_cnpollc(dev_t, int);
void milkymist_com_start(struct tty *);
int milkymist_com_param(struct tty *, struct termios *);

static struct cnm_state milkymist_com_cnm_state;

CFATTACH_DECL_NEW(milkymist_com, sizeof(struct milkymist_com_softc),
    uart_match, uart_attach, NULL, NULL);

dev_type_open(milkymist_com_open);
dev_type_write(milkymist_com_write);

/* char dev switch structure for tty */
const struct cdevsw milkymist_com_cdevsw = 
{
    milkymist_com_open,
    NULL,
    NULL,
    milkymist_com_write,
    NULL,
    NULL,
    NULL,
    NULL,
    nommap,             /* mmap not supported */
    ttykqfilter,
    D_TTY
};

/************************************************************** 
 * cdevsw functions
 */

int
milkymist_com_open(dev_t dev, int flag, int mode, struct lwp *l)
{
        struct milkymist_com_softc *sc = uart_sc;
        struct tty *tp = sc->sc_tty;
        int s, error = 0;

        s = spltty();

        tp->t_dev = dev;
        if ((tp->t_state & TS_ISOPEN) == 0) {
                tp->t_state |= TS_CARR_ON;
                ttychars(tp);
                tp->t_iflag = TTYDEF_IFLAG;
                tp->t_oflag = TTYDEF_OFLAG;
                tp->t_cflag = TTYDEF_CFLAG | CLOCAL;
                tp->t_lflag = TTYDEF_LFLAG;
                tp->t_ispeed = tp->t_ospeed = 115200;
                ttsetwater(tp);
        } else if (kauth_authorize_device_tty(l->l_cred, KAUTH_DEVICE_TTY_OPEN,
            tp) != 0) {
                splx(s);
                return (EBUSY);
        }

        splx(s);

        error = (*tp->t_linesw->l_open)(dev, tp);

        return (error);
}

int
milkymist_com_write(dev_t dev, struct uio *uio, int flag)
{
	struct milkymist_com_softc *sc = uart_sc;
	struct tty *tp = sc->sc_tty;

	return ((*tp->t_linesw->l_write)(tp, uio, flag));
}

int
milkymist_com_param(struct tty *tp, struct termios *t)
{

        return (0);
}

void
milkymist_com_start(struct tty *tp)
{
        int s,i,cnt;

        s = spltty();
        if (tp->t_state & (TS_TTSTOP | TS_BUSY))
                goto out;
        ttypull(tp);
        tp->t_state |= TS_BUSY;
        while (tp->t_outq.c_cc != 0) {
                cnt = ndqb(&tp->t_outq, 0);
                for (i=0; i<cnt; i++)
                        milkymist_com_cnputc(0,tp->t_outq.c_cf[i]);
                ndflush(&tp->t_outq, cnt);
        }
        tp->t_state &= ~TS_BUSY;
 out:
        splx(s);
}

int
uart_match(device_t parent, cfdata_t cf, void *aux)
{
	return 1;
}

void
uart_attach(device_t parent, device_t self, void *aux)
{
	int maj, mm_min;
	struct tty *tp;
	struct milkymist_com_softc *sc = device_private(self);

	if (uart_sc == NULL)
		uart_sc = sc;

	sc->sc_dev = self;

	maj = cdevsw_lookup_major(&milkymist_com_cdevsw);
	cn_tab->cn_dev = makedev(maj, 0);
	aprint_normal(" major = %i: console\n", maj);
	mm_min = device_unit(sc->sc_dev);

	tp = tty_alloc();
	if (tp == NULL)
		panic("tty NULL\n");
	tp->t_oproc = milkymist_com_start;
	tp->t_param = milkymist_com_param;
	sc->sc_tty = tp;
	tp->t_dev = makedev(maj, mm_min);
	tty_attach(tp);
	cn_tab->cn_dev = tp->t_dev;
}

#define UART_STAT_RX_EVT (0x2)
#define UART_STAT_THRE (0x1)

//TODO: implement com_cngetc
static int milkymist_com_cngetc(dev_t dev)
{
  volatile unsigned int *uart_rxtx_buff = (volatile unsigned int *)uart_base_vaddr;
  volatile unsigned int *uart_stat = (volatile unsigned int *)uart_base_vaddr + 2;
  int s;
  int c;

  s = splhigh();
  while ( !(*uart_stat & UART_STAT_RX_EVT));
  c = *uart_rxtx_buff;
  *uart_stat = UART_STAT_RX_EVT;
  splx(s);

  return c;
}

unsigned int uart_base_vaddr = kern_phy_to_virt(0xe0000000);

static void milkymist_com_cnputc(dev_t dev, int c)
{
  volatile unsigned int *uart_rxtx_buff = (volatile unsigned int *)uart_base_vaddr;
  volatile unsigned int *uart_stat = (volatile unsigned int *)uart_base_vaddr + 2;
	int s;
	s = splhigh();
	while (!(UART_STAT_THRE & *uart_stat));
	*uart_rxtx_buff = c;
	splx(s);
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
