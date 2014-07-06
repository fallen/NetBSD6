/*	$NetBSD: $	*/

/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <lm32/cpu.h>
#include <lm32/pmap.h>
#include <lm32/vmparam.h>
#include <sys/lwp.h>
#include <sys/errno.h>

int copyout(const void *kaddr, void *uaddr, size_t len);
int copyin(const void *uaddr, void *kaddr, size_t len);
int copyinstr(const void *from, void *to, size_t max_len, size_t *len_copied);
int copystr(const void *kfaddr, void *kdaddr, size_t maxlen, size_t *done);

int copyin(const void *uaddr, void *kaddr, size_t len)
{
	const uint32_t *uaddr_32 = uaddr;
	const uint8_t *uaddr_8 = uaddr;
	uint32_t *kaddr_32 = kaddr;
	uint8_t *kaddr_8 = kaddr;
	int count;
	uint8_t remainder;

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

	remainder = len - count;
	kaddr_8 -= count;
	uaddr_8 += remainder;

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
	uint8_t remainder;

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

	remainder = len - count;
	uaddr_8 -= count;
	kaddr_8 += remainder;

	while (count-- > 0)
		*uaddr_8++ = *kaddr_8++;

	return 0;
}

int copyinstr(const void *from, void *to, size_t max_len, size_t *len_copied)
{
	const char *from_8 = (const char *)from;
	char *to_8 = (char *)to;
	int ret = 0;
	size_t maxlen_temp = max_len;

	if ((size_t)from > VM_MAXUSER_ADDRESS) /* to be checked, jump if carry ? */
	{
		ret = EFAULT;
		goto copyinstr_return;
	}

	if (max_len > VM_MAXUSER_ADDRESS - (size_t)from)
		max_len = VM_MAXUSER_ADDRESS - (size_t)from;

	maxlen_temp++;

	do
	{
		if (--maxlen_temp == 0)
		{
			ret = 1;
			break;
		}
		*to_8++ = *from_8;
	} while (*from_8++ != '\0');


	if (ret)
	{
		if (VM_MAXUSER_ADDRESS >= (size_t)from_8)
			ret = EFAULT;
		else
			ret = ENAMETOOLONG;
	}
	else
	{
		maxlen_temp--; 
		ret = 0;
	}

copyinstr_return:
	if (NULL != len_copied)
		*len_copied = max_len - maxlen_temp;

	return ret;
}


int copystr(const void *kfaddr, void *kdaddr, size_t maxlen, size_t *done)
{
	const char *from_8 = kfaddr;
	char *to_8 = kdaddr;
	size_t maxlen_temp = maxlen;
	int ret = 0;

	from_8--;
	maxlen_temp++;

	do
	{
		if (--maxlen_temp == 0)
		{
			ret = ENAMETOOLONG;
			break;
		}
		*to_8++ = *from_8;
	} while (*from_8++ != '\0');

	if (ret == 0)
	{
		maxlen_temp--;
	} else
		ret = ENAMETOOLONG;

	if (NULL != done)
		*done = maxlen - maxlen_temp;

	return ret;
}

int copyoutstr(const void *from, void *to, size_t maxlen, size_t *done)
{
	return 0;
	//TODO
}
