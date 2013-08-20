#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: timer.c,v 1.11 2010/11/21 18:53:56 ysionneau Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <machine/autoconf.h>

static int timer_match(device_t, cfdata_t, void*);
static void timer_attach(device_t, device_t, void *);

CFATTACH_DECL_NEW(timer, 0,
    timer_match, timer_attach, NULL, NULL);

int
timer_match(device_t parent, cfdata_t cf, void *aux)
{
	return 1;
}

void
timer_attach(device_t parent, device_t self, void *aux)
{
	aprint_normal("\n");
}
