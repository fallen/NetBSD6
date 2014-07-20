/*	$NetBSD: $	*/
/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/asm.h>
#include "assym.h"

// TODO: implement suword
ENTRY(suword)
	ret

// TODO: implement susword
ENTRY(susword)
	ret

// TODO: implement suswintr
ENTRY(suswintr)
	ret

// TODO: implement subyte
ENTRY(subyte)
	ret

// TODO: implement fuswintr
ENTRY(fuswintr)
	ret

// TODO: implement setjmp
ENTRY(setjmp)
	ret

// TODO: implement longjmp
ENTRY(longjmp)
	ret

/* setfault(struct trapframe *tf) */
/* actually arg1 is faultframe but on lm32 it's the same as trapframe */
ENTRY(setfault)
	addi	sp, sp, -4
	sw	(sp+0), r25

	mvhi	r25, hi(_C_LABEL(cpu_info_store))
	ori	r25, r25, lo(_C_LABEL(cpu_info_store))
	lw	r25, (r25+CPU_INFO_CURLWP)
	lw	r25, (r25+L_PCB)
	sw	(r25+PCB_ONFAULT), r1

	lw	r25, (sp+0)

	sw	(r1+4*_REG_R1), r1
	sw	(r1+4*_REG_R2), r2
	sw	(r1+4*_REG_R3), r3
	sw	(r1+4*_REG_R4), r4
	sw	(r1+4*_REG_R5), r5
	sw	(r1+4*_REG_R6), r6
	sw	(r1+4*_REG_R7), r7
	sw	(r1+4*_REG_R8), r8
	sw	(r1+4*_REG_R9), r9
	sw	(r1+4*_REG_R10), r10
	sw	(r1+4*_REG_R11), r11
	sw	(r1+4*_REG_R12), r12
	sw	(r1+4*_REG_R13), r13
	sw	(r1+4*_REG_R14), r14
	sw	(r1+4*_REG_R15), r15
	sw	(r1+4*_REG_R16), r16
	sw	(r1+4*_REG_R17), r17
	sw	(r1+4*_REG_R18), r18
	sw	(r1+4*_REG_R19), r19
	sw	(r1+4*_REG_R20), r20
	sw	(r1+4*_REG_R21), r21
	sw	(r1+4*_REG_R22), r22
	sw	(r1+4*_REG_R23), r23
	sw	(r1+4*_REG_R24), r24
	sw	(r1+4*_REG_R25), r25
	sw	(r1+4*_REG_GP), gp
	sw	(r1+4*_REG_FP), fp
	sw	(r1+4*_REG_SP), sp
	sw	(r1+4*_REG_RA), ra
	sw	(r1+4*_REG_EA), ea
	sw	(r1+4*_REG_BA), ba

	xor	r1, r1, r1
	addi	sp, sp, 4
	ret

ENTRY(lwp_trampoline)

	calli	_C_LABEL(lwp_startup)

	GET_CPUVAR(r11, CURLWP)
	lw	r11, (r11+L_PCB)
	lw	r1, (r11+PCB_REGS+4*_REG_R1)
	lw	r2, (r11+PCB_REGS+4*_REG_R2)
	lw	r25, (r11+PCB_REGS+4*_REG_R25) /* R25 is overwritten by GET_CPUVAR */
	lw	r11, (r11+PCB_REGS+4*_REG_R11)

	call	r2

/* from this point, we return to user space.
 * setregs has been called to set all regs to 0 and pc to entry point */

	GET_CPUVAR(r1, CURLWP)
	lw	r1, (r1+L_MD_UTF)
	lw	r3, (r1+TF_REGS+4*_REG_R3)
	lw	r4, (r1+TF_REGS+4*_REG_R4)
	lw	r5, (r1+TF_REGS+4*_REG_R5)
	lw	r6, (r1+TF_REGS+4*_REG_R6)
	lw	r7, (r1+TF_REGS+4*_REG_R7)
	lw	r8, (r1+TF_REGS+4*_REG_R8)
	lw	r9, (r1+TF_REGS+4*_REG_R9)
	lw	r10, (r1+TF_REGS+4*_REG_R10)
	lw	r11, (r1+TF_REGS+4*_REG_R11)
	lw	r12, (r1+TF_REGS+4*_REG_R12)
	lw	r13, (r1+TF_REGS+4*_REG_R13)
	lw	r14, (r1+TF_REGS+4*_REG_R14)
	lw	r15, (r1+TF_REGS+4*_REG_R15)
	lw	r16, (r1+TF_REGS+4*_REG_R16)
	lw	r17, (r1+TF_REGS+4*_REG_R17)
	lw	r18, (r1+TF_REGS+4*_REG_R18)
	lw	r19, (r1+TF_REGS+4*_REG_R19)
	lw	r20, (r1+TF_REGS+4*_REG_R20)
	lw	r21, (r1+TF_REGS+4*_REG_R21)
	lw	r22, (r1+TF_REGS+4*_REG_R22)
	lw	r23, (r1+TF_REGS+4*_REG_R23)
	lw	r24, (r1+TF_REGS+4*_REG_R24)
	lw	r25, (r1+TF_REGS+4*_REG_R25)
	lw	gp, (r1+TF_REGS+4*_REG_GP)
	lw	fp, (r1+TF_REGS+4*_REG_FP)
	lw	sp, (r1+TF_REGS+4*_REG_SP)
	lw	ra, (r1+TF_REGS+4*_REG_RA)
	lw	ea, (r1+TF_REGS+4*_REG_PC)
	lw	ba, (r1+TF_REGS+4*_REG_BA)

	/* set PSW.EUSR bit: PSW |= PSW_EUSR;  */
	rcsr	r2, PSW
	ori	r2, r2, PSW_EUSR
	wcsr	PSW, r2

	lw	r2, (r1+TF_REGS+4*_REG_R2)
	lw	r1, (r1+TF_REGS+4*_REG_R1)
	eret

/*
 * struct lwp *cpu_switchto(struct lwp *oldlwp, struct *newlwp,
 *			    bool returning)
 *
 *	1. if (oldlwp != NULL), save its context.
 *	2. then, restore context of newlwp.
 *
 * Note that the stack frame layout is known to "struct switchframe" in
 * <machine/frame.h> and to the code in cpu_lwp_fork() which initializes
 * it for a new lwp.
 * r1: oldlwp and return value
 * r2: newlwp
 * r3: returning
 */
ENTRY(cpu_switchto)
	addi	sp, sp, -12
	sw	(sp+0), ra
	sw	(sp+4), r11
	sw	(sp+8), r12

	be	r1, r0, 1f

	/* Save old context. */
	lw	r11, (r1+L_PCB)
	sw	(r11+PCB_REGS+4*_REG_R1),  r1
	sw	(r11+PCB_REGS+4*_REG_R2),  r2
	sw	(r11+PCB_REGS+4*_REG_R3),  r3
	sw	(r11+PCB_REGS+4*_REG_R4),  r4
	sw	(r11+PCB_REGS+4*_REG_R5),  r5
	sw	(r11+PCB_REGS+4*_REG_R6),  r6
	sw	(r11+PCB_REGS+4*_REG_R7),  r7
	sw	(r11+PCB_REGS+4*_REG_R8),  r8
	sw	(r11+PCB_REGS+4*_REG_R9),  r9
	sw	(r11+PCB_REGS+4*_REG_R10), r10
	sw	(r11+PCB_REGS+4*_REG_R12), r12
	sw	(r11+PCB_REGS+4*_REG_R13), r13
	sw	(r11+PCB_REGS+4*_REG_R14), r14
	sw	(r11+PCB_REGS+4*_REG_R15), r15
	sw	(r11+PCB_REGS+4*_REG_R16), r16
	sw	(r11+PCB_REGS+4*_REG_R17), r17
	sw	(r11+PCB_REGS+4*_REG_R18), r18
	sw	(r11+PCB_REGS+4*_REG_R19), r19
	sw	(r11+PCB_REGS+4*_REG_R20), r20
	sw	(r11+PCB_REGS+4*_REG_R21), r21
	sw	(r11+PCB_REGS+4*_REG_R22), r22
	sw	(r11+PCB_REGS+4*_REG_R23), r23
	sw	(r11+PCB_REGS+4*_REG_R24), r24
	sw	(r11+PCB_REGS+4*_REG_R25), r25
	sw	(r11+PCB_REGS+4*_REG_GP),  gp
	sw	(r11+PCB_REGS+4*_REG_FP),  fp
	sw	(r11+PCB_REGS+4*_REG_SP),  sp
	sw	(r11+PCB_REGS+4*_REG_RA),  ra
	sw	(r11+PCB_REGS+4*_REG_EA),  ea
	sw	(r11+PCB_REGS+4*_REG_BA),  ba

	lw	r12, (sp+4) /* load r11 to r12 */
	sw	(r11+PCB_REGS+4*_REG_R11), r12 /* save r11 as well */

	/* Switch to newlwp's stack. */
1:	
	lw	r12, (r2+L_PCB)

	/* Set curlwp. */
	SET_CPUVAR(CURLWP, r2)

	/* Set cpu_info_store.ci_curpm to newlwp->l_proc->p_vmspace->vm_map.pmap*/
	lw	r4, (r2+L_PROC)
	lw	r4, (r4+P_VMSPACE)
	lw	r4, (r4+VM_MAP+VM_MAP_PMAP)
	SET_CPUVAR(CURPM, r4)

	/* Skip the rest if returning to a pinned LWP. */
	bne	r3, r0, 4f /* branch if r3 != 0 */

	/* Check if this process is using RAS (restartable atomic sequence). */
	/* TODO: check for RAS */
	lw	r12, (r2+L_PROC)
	lw	r12, (r12+P_RASLIST)
	bne	r12, r0, 5f
	
	/* Return to the new LWP, returning 'oldlwp' in r1. */
4:

	/* restore new LWP registers */
  
	lw	r11, (r2+L_PCB)
	lw	r3,  (r11+PCB_REGS+4*_REG_R3)
	lw	r4,  (r11+PCB_REGS+4*_REG_R4)
	lw	r5,  (r11+PCB_REGS+4*_REG_R5)
	lw	r6,  (r11+PCB_REGS+4*_REG_R6)
	lw	r7,  (r11+PCB_REGS+4*_REG_R7)
	lw	r8,  (r11+PCB_REGS+4*_REG_R8)
	lw	r9,  (r11+PCB_REGS+4*_REG_R9)
	lw	r10, (r11+PCB_REGS+4*_REG_R10)
	lw	r12, (r11+PCB_REGS+4*_REG_R12)
	lw	r13, (r11+PCB_REGS+4*_REG_R13)
	lw	r14, (r11+PCB_REGS+4*_REG_R14)
	lw	r15, (r11+PCB_REGS+4*_REG_R15)
	lw	r16, (r11+PCB_REGS+4*_REG_R16)
	lw	r17, (r11+PCB_REGS+4*_REG_R17)
	lw	r18, (r11+PCB_REGS+4*_REG_R18)
	lw	r19, (r11+PCB_REGS+4*_REG_R19)
	lw	r20, (r11+PCB_REGS+4*_REG_R20)
	lw	r21, (r11+PCB_REGS+4*_REG_R21)
	lw	r22, (r11+PCB_REGS+4*_REG_R22)
	lw	r23, (r11+PCB_REGS+4*_REG_R23)
	lw	r24, (r11+PCB_REGS+4*_REG_R24)
	lw	r25, (r11+PCB_REGS+4*_REG_R25)
	lw	gp,  (r11+PCB_REGS+4*_REG_GP)
	lw	fp,  (r11+PCB_REGS+4*_REG_FP)
	lw	sp,  (r11+PCB_REGS+4*_REG_SP)
	lw	ra,  (r11+PCB_REGS+4*_REG_RA)
	lw	ea,  (r11+PCB_REGS+4*_REG_EA)
	lw	ba,  (r11+PCB_REGS+4*_REG_BA)

	mv	r0, r11
	lw	r11, (r0+PCB_REGS+4*_REG_R11) /* restore r11 as well */
	xor	r0, r0, r0

	lw	r12, (sp+8)
	lw	r11, (sp+4)
	addi	sp, sp, 12
	ret

	/* Check for restartable atomic sequence (RAS). */
5:
	sw	(sp+0), r1
	sw	(sp+4), r2
	addi	sp, sp, 8
	lw	r2, (r2+L_MD_UTF)
	lw	r2, (r2+TF_REGS+4*_REG_PC)
	lw	r1, (sp+4)
	lw	r1, (r1+L_PROC)

	/* call ras_lookup(newlwp->l_proc, newlwp.md_regs.md_utf.tf_regs.r_pc); */
	calli	_C_LABEL(ras_lookup)

	lw	r2, (sp+4)

	/* save return value in tf_regs.r_pc */
	lw	r2, (r2+L_MD_UTF)
	sw	(r2+TF_REGS+4*_REG_PC), r1

	lw	r2, (sp+4)
	lw	r1, (sp+0)
	addi	sp, sp, -8
	bi	4b
