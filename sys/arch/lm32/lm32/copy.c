/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/cpu.h>
#include <lm32/pmap.h>
#include <lm32/vmparam.h>
#include <sys/lwp.h>
#include <sys/errno.h>

void do_pmap_load(void);
int copyout(const void *kaddr, void *uaddr, size_t len);
int copyin(const void *uaddr, void *kaddr, size_t len);

void do_pmap_load(void)
{
	struct lwp *current_lwp = curcpu()->ci_curlwp;

	do {
		current_lwp->l_nopreempt++;
		pmap_load();
		current_lwp->l_nopreempt--;

		if (current_lwp->l_nopreempt == 0)
		{
			if (current_lwp->l_dopreempt != 0)
				kpreempt(0);
		}

	} while (curcpu()->ci_want_pmapload != 0);

}

int copyin(const void *uaddr, void *kaddr, size_t len)
{
	const uint32_t *uaddr_32 = uaddr;
	const uint8_t *uaddr_8 = uaddr;
	uint32_t *kaddr_32 = kaddr;
	uint8_t *kaddr_8 = kaddr;
	int count;

	if ( curcpu()->ci_want_pmapload )
		do_pmap_load();

	kaddr_8 += len;
	if ((size_t)kaddr_8 < len)
		return EFAULT;
	if ((size_t)uaddr_8 > VM_MAXUSER_ADDRESS)
		return EFAULT;

	count = len;
	count >>= 2;     /* count = count / 4 */
	while ( count-- > 0)
		*kaddr_32++ = *uaddr_32++;

	count = len & 0x3;  /* test if it is a multiple of 4 */
	if (count == 0)
		return 0;

	kaddr_8 = (uint8_t *)kaddr + len - count;

	while (count-- > 0)
		*kaddr_8++ = *uaddr_8++;

	return 0;
}

int copyout(const void *kaddr, void *uaddr, size_t len)
{
	uint32_t *uaddr_32 = uaddr;
	uint8_t *uaddr_8 = uaddr;
	const uint32_t *kaddr_32 = kaddr;
	const uint8_t *kaddr_8 = kaddr;
	int count;

	if ( curcpu()->ci_want_pmapload )
		do_pmap_load();

	uaddr_8 += len;
	if ((size_t)uaddr_8 < len) /* is it correct as an overflow condition ? */
		return EFAULT;
	if ((size_t)uaddr_8 > VM_MAXUSER_ADDRESS)
		return EFAULT;

	count = len;
	count >>= 2;     /* count = count / 4 */
	while ( count-- > 0)
		*uaddr_32++ = *kaddr_32++;

	count = len & 0x3;  /* test if it is a multiple of 4 */
	if (count == 0)
		return 0;

	uaddr_8 = (uint8_t *)uaddr + len - count;

	while (count-- > 0)
		*uaddr_8++ = *kaddr_8++;

	return 0;
}
