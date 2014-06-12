/*
 * COPYRIGHT (C) 2014 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <sys/param.h>
#include <uvm/uvm.h>

void lm32_trap(unsigned int vaddr, unsigned int *ctx, unsigned int return_address);

void lm32_trap(unsigned int vaddr, unsigned int *ctx, unsigned int return_address)
{
  int ret;
  unsigned int psw;

  if ((unsigned int *)vaddr == NULL)
    panic("NULL pointer dereferencement");

  //FIXME: put the real fault type instead of VM_PROT_READ
  ret = uvm_fault(kernel_map, vaddr, VM_PROT_READ);

  if (ret != 0)
  {
    ret = uvm_fault(&curlwp->l_proc->p_vmspace->vm_map, vaddr, VM_PROT_READ);
    if (ret != 0)
      panic("cannot resolve page fault");
  }

  asm volatile("rcsr %0, PSW" : "=r"(psw) :: );

  psw &= ~(PSW_IE_EIE | PSW_EDTLBE | PSW_EITLBE);

  asm volatile("wcsr PSW, %0" :: "r"(psw) : );

  asm volatile("mv ea, %0\n\t"
               "eret" :: "r"(return_address) : );
}

