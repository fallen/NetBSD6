/*
 * LatticeMico32 C startup code.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <lm32/asm.h>

/* Exception handlers - Must be 32 bytes long. */
.section    .text, "ax", @progbits
.global _start
.type _start,@function
.global start
.type start,@function

start:
_bootstrap:
	xor	r0, r0, r0
  wcsr  PSW, r0
/*mvhi	r1, hi(_reset_handler - _bootstrap)
	ori	r1, r1, lo(_reset_handler) */
	mvhi	r1, 0x4000
	ori	r1, r1, lo(_start)
	wcsr	EBA, r1
	xor	r2, r2, r2
	bi	_crt0
	nop

_memory_store_area:
.word _memory_store_area - start + 0x40000000 +8

.org 0x1000 /* we reserve one page of memory storage to save registers during (nested) exceptions */

_start:
kernel_text:
_reset_handler:
	xor	r0, r0, r0
	wcsr	PSW, r0
	mvhi	r1, hi(_reset_handler)
	ori	r1, r1, lo(_reset_handler)
	wcsr	EBA, r1
	xor	r2, r2, r2
	bi	_crt0
	nop

_breakpoint_handler:
	bi _breakpoint_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_instruction_bus_error_handler:
	bi _instruction_bus_error_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_watchpoint_hander:
	bi _watchpoint_hander
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_data_bus_error_handler:
	bi _data_bus_error_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_divide_by_zero_handler:
	bi _divide_by_zero_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_interrupt_handler:
  bi _real_interrupt_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_syscall_handler:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_ENTRY(_itlb_miss_handler)
	bi	_fake_itlb_miss_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_ENTRY(_dtlb_miss_handler)
	bi	_fake_dtlb_miss_handler
	nop
	nop
	nop
	nop
	nop
	nop
	nop

_ENTRY(_dtlb_fault_handler)
	rcsr	r0, TLBPADDR
	ori	r0, r0, 1
	wcsr	TLBPADDR, r0
	xor	r0, r0, r0 /* restore r0 to 0 */
	eret
	nop
	nop
	nop


_privilege_fault_handler:
	eret
	nop
	nop
	nop
	nop
	nop
	nop
	nop

macaddress:
	.byte 0x10
	.byte 0xe2
	.byte 0xd5
	.byte 0x00
	.byte 0x00
	.byte 0x00

	/* padding to align to a 32-bit boundary */
	.byte 0x00
	.byte 0x00

_ENTRY(_fake_itlb_miss_handler)
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw  r0, (r0+0)
	sw	(r0+0), r1
	sw	(r0+4), r2
  lw  r1, (r0+-4) /* r1 = memory_store_area.first_jump_to_vaddr_done */
	xor	r0, r0, r0 /* restore r0 to 0 */
  be  r1, r0, 1f

  rcsr r1, TLBPADDR

  mvhi r2, 0xffff
  ori r2, r2, 0xf000
  and r1, r1, r2 /* r1 &= ~(PG_MASK) */  
  mvhi r2, 0xc000
  sub r1, r1, r2
  mvhi r2, 0x4000
  add r1, r1, r2
 
  wcsr TLBPADDR, r1

	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw  r0, (r0+0)
	lw	r1, (r0+0)
	lw	r2, (r0+4)
	xor	r0, r0, r0 /* restore r0 to 0 */
  eret

1:
  mvhi r1, 0x4000
  sub ea, ea, r1
  mvhi r1, 0xc000
  add ea, ea, r1
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  ori r2, r2, 0x42
  lw  r0, (r0+0)
	sw	(r0+-4), r2 /* memory_store_area.first_jump_to_vaddr_done = true */
	lw	r1, (r0+0)
	lw	r2, (r0+4)
	xor	r0, r0, r0 /* restore r0 to 0 */
  eret

_ENTRY(_fake_dtlb_miss_handler)
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw  r0, (r0+0)
	sw	(r0+0), r1
	sw	(r0+4), r2
	xor	r0, r0, r0 /* restore r0 to 0 */

  rcsr r1, TLBPADDR

  mvhi r2, 0xffff
  ori r2, r2, 0xf000
  and r1, r1, r2 /* r1 &= ~(PG_MASK) */  
  mvhi r2, 0xc000
  sub r1, r1, r2
  mvhi r2, 0x4000
  add r1, r1, r2
  ori r1, r1, 1 /* writting to DTLB */ 
  wcsr TLBPADDR, r1

	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw  r0, (r0+0)
	lw	r1, (r0+0)
	lw	r2, (r0+4)
	xor	r0, r0, r0 /* restore r0 to 0 */
  eret

_ENTRY(_real_interrupt_handler)
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw  r0, (r0+0)
	sw	(r0+0), r1
	sw	(r0+4), r2
	sw	(r0+8), r3
	sw	(r0+12), r4
	sw	(r0+16), r5
	sw	(r0+20), r6
	sw	(r0+24), r7
	sw	(r0+28), r8
	sw	(r0+32), r9
	sw	(r0+36), r10
	sw	(r0+40), r11
	sw	(r0+44), r12
	sw	(r0+48), r13
	sw	(r0+52), r14
	sw	(r0+56), r15
	sw	(r0+60), r16
	sw	(r0+64), r17
	sw	(r0+68), r18
	sw	(r0+72), r19
	sw  (r0+76), r20
	sw  (r0+80), r21
  sw  (r0+84), r22
  sw  (r0+88), r23
  sw  (r0+92), r24
  sw  (r0+96), r25
  sw  (r0+100), gp
  sw  (r0+104), fp
  sw  (r0+108), sp
	sw  (r0+112), ea
	sw  (r0+116), ba
	sw  (r0+120), ra
  rcsr r3, PSW
  sw  (r0+124), r3
	xor	r0, r0, r0 /* restore r0 value to 0 */
  /* now update memory_store_area in case of nested tlb miss */
  mvhi r1, 0x4000
  ori r1, r1, lo(_memory_store_area)
  lw r2, (r1+0)
  mv r5, r2
  addi r2, r2, 128
  sw (r1+0), r2

  rcsr r1, IP
	mvhi	ea, hi(__isr) /* function we want to call */
	ori	ea, ea, lo(__isr)
  mvhi r2, hi(1f)                          /* where we want to return back to */
  ori r2, r2, lo(1f)
  mvhi  r3, 0xc000
  sub r2, r2, r3
  mvhi r3, 0x4000
  add r2, r2, r3
  xor r3, r3, r3
  ori r3, r3, 0x90 /* PSW_EDTLBE | PSW_EITLBE */
  wcsr PSW, r3
  mv r3, r5 /* arg3 is trapframe */
  /* we then use eret as a trick to call __isr
  * with TLB ON and interrupts off */
  eret
  

1:
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw r1, (r0+0)
  addi r1, r1, -128
  sw (r0+0), r1
  addi r0, r1, 0 /* we cannot use 'mv' when r0 != 0 */
	lw	r1, (r0+124)
  wcsr PSW, r1
	lw	r1, (r0+0)
	lw	r2, (r0+4)
	lw	r3, (r0+8)
	lw	r4, (r0+12)
	lw	r5, (r0+16)
	lw	r6, (r0+20)
	lw	r7, (r0+24)
	lw	r8, (r0+28)
	lw	r9, (r0+32)
	lw	r10, (r0+36)
	lw	r11, (r0+40)
	lw	r12, (r0+44)
	lw	r13, (r0+48)
	lw	r14, (r0+52)
	lw	r15, (r0+56)
	lw	r16, (r0+60)
	lw	r17, (r0+64)
	lw	r18, (r0+68)
	lw	r19, (r0+72)
	lw	r20, (r0+76)
	lw	r21, (r0+80)
	lw	r22, (r0+84)
	lw	r23, (r0+88)
	lw	r24, (r0+92)
	lw	r25, (r0+96)
	lw	gp, (r0+100)
	lw	fp, (r0+104)
  lw  sp, (r0+108)
	lw	ea, (r0+112)
	lw	ba, (r0+116)
	lw	ra, (r0+120)
	xor	r0, r0, r0 /* restore r0 value to 0 */
  eret

_ENTRY(_real_tlb_miss_handler)
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw  r0, (r0+0)
	sw	(r0+0), r1
	sw	(r0+4), r2
	sw	(r0+8), r3
	sw	(r0+12), r4
	sw	(r0+16), r5
	sw	(r0+20), r6
	sw	(r0+24), r7
	sw	(r0+28), r8
	sw	(r0+32), r9
	sw	(r0+36), r10
	sw	(r0+40), r11
	sw	(r0+44), r12
	sw	(r0+48), r13
	sw	(r0+52), r14
	sw	(r0+56), r15
	sw	(r0+60), r16
	sw	(r0+64), r17
	sw	(r0+68), r18
	sw	(r0+72), r19
	sw  (r0+76), r20
	sw  (r0+80), r21
  sw  (r0+84), r22
  sw  (r0+88), r23
  sw  (r0+92), r24
  sw  (r0+96), r25
  sw  (r0+100), gp
  sw  (r0+104), fp
  sw  (r0+108), sp
	sw  (r0+112), ea
	sw  (r0+116), ba
	sw  (r0+120), ra
  rcsr r3, PSW
  sw  (r0+124), r3
	xor	r0, r0, r0 /* restore r0 value to 0 */
  /* now update memory_store_area in case of nested tlb miss */
  mvhi r1, 0x4000
  ori r1, r1, lo(_memory_store_area)
  lw r2, (r1+0)
  addi r2, r2, 128
  sw (r1+0), r2

  rcsr r1, TLBVADDR
  rcsr r2, TLBPADDR
  andi r3, r3, 0x400
  bne r3, r0, we_come_from_user_space
  mvhi r3, 0xc800
  cmpgeu r4, r1, r3
  bne r4, r0, out_of_ram_window
  mvhi r3, 0xc000
  cmpgu r4, r3, r1
  bne r4, r0, out_of_ram_window
  mvhi r3, 0xc000
  sub r1, r1, r3
  mvhi r3, 0x4000
  add r1, r1, r3
  wcsr TLBPADDR, r1
  bi 1f /* let's return to what we were doing */

we_come_from_user_space:
out_of_ram_window:
  mv  r3, ea /* ea is passed as 3rd argument to _do_real_tlb_miss_handling */
	mvhi	ea, hi(_do_real_tlb_miss_handling) /* function we want to call */
	ori	ea, ea, lo(_do_real_tlb_miss_handling)
  mvhi ra, hi(1f)                          /* where we want to return back to */
  ori ra, ra, lo(1f)
  mvhi  r4, 0xc000
  sub ra, ra, r4
  mvhi r4, 0x4000
  add ra, ra, r4
  xor r4, r4, r4
  ori r4, r4, 0x90 /* PSW_EDTLBE | PSW_EITLBE */
  wcsr PSW, r4
  /* we then use eret as a trick to call _do_real_tlb_miss_handling
  * with TLB ON */
  eret

1:
	mvhi	r0, 0x4000
	ori	r0, r0, lo(_memory_store_area)
  lw r1, (r0+0)
  addi r1, r1, -128
  sw (r0+0), r1
  addi r0, r1, 0 /* we cannot use 'mv' when r0 != 0 */
	lw	r1, (r0+124)
  wcsr PSW, r1
	lw	r1, (r0+0)
	lw	r2, (r0+4)
	lw	r3, (r0+8)
	lw	r4, (r0+12)
	lw	r5, (r0+16)
	lw	r6, (r0+20)
	lw	r7, (r0+24)
	lw	r8, (r0+28)
	lw	r9, (r0+32)
	lw	r10, (r0+36)
	lw	r11, (r0+40)
	lw	r12, (r0+44)
	lw	r13, (r0+48)
	lw	r14, (r0+52)
	lw	r15, (r0+56)
	lw	r16, (r0+60)
	lw	r17, (r0+64)
	lw	r18, (r0+68)
	lw	r19, (r0+72)
	lw	r20, (r0+76)
	lw	r21, (r0+80)
	lw	r22, (r0+84)
	lw	r23, (r0+88)
	lw	r24, (r0+92)
	lw	r25, (r0+96)
	lw	gp, (r0+100)
	lw	fp, (r0+104)
  lw  sp, (r0+108)
	lw	ea, (r0+112)
	lw	ba, (r0+116)
	lw	ra, (r0+120)
	xor	r0, r0, r0 /* restore r0 value to 0 */
	eret

_crt0:
  mvhi  r1, hi(_memory_store_area)
  ori r1, r1, lo(_memory_store_area)
  lw  r2, (r1+0)
  addi  r2, r2, 4
  sw (r1+0), r2
	/* activate ITLB and DTLB */
	mvi	r1, 0x48
	wcsr 	PSW, r1

	/* stack and global pointers 
   * should be initialized
   * by bootloader/BIOS
   */
	/* Setup stack and global pointer */
	mvhi    sp, hi(_fstack)
	ori     sp, sp, lo(_fstack)
	mvhi    gp, hi(_gp)
	ori     gp, gp, lo(_gp)

	/* Clear BSS */
	mvhi    r1, hi(_fbss)
	ori     r1, r1, lo(_fbss)
	mvhi    r3, hi(_ebss)
	ori     r3, r3, lo(_ebss)
.clearBSS:
	be      r1, r3, .callMain
	sw      (r1+0), r0
	addi    r1, r1, 4
	bi      .clearBSS

.callMain:	
	mv      r1, r2
	mvi     r2, 0
	mvi     r3, 0
	mvhi	r4, hi(milkymist_startup)
	ori	r4, r4, lo(milkymist_startup)
	b	r4

.save_all:
	addi    sp, sp, -56
	sw      (sp+4), r1
	sw      (sp+8), r2
	sw      (sp+12), r3
	sw      (sp+16), r4
	sw      (sp+20), r5
	sw      (sp+24), r6
	sw      (sp+28), r7
	sw      (sp+32), r8
	sw      (sp+36), r9
	sw      (sp+40), r10
	sw      (sp+48), ea
	sw      (sp+52), ba
	/* ra needs to be moved from initial stack location */
	lw      r1, (sp+56)
	sw      (sp+44), r1
	ret

.restore_all_and_eret:
	lw      r1, (sp+4)
	lw      r2, (sp+8)
	lw      r3, (sp+12)
	lw      r4, (sp+16)
	lw      r5, (sp+20)
	lw      r6, (sp+24)
	lw      r7, (sp+28)
	lw      r8, (sp+32)
	lw      r9, (sp+36)
	lw      r10, (sp+40)
	lw      ra, (sp+44)
	lw      ea, (sp+48)
	lw      ba, (sp+52)
	addi    sp, sp, 56
	eret
