/*
 * COPYRIGHT (C) 2013 Yann Sionneau <yann.sionneau@gmail.com>
 */

#include <lm32/asm.h>
#include "assym.h"

// TODO: implement suword
ENTRY(suword)

// TODO: implement susword
ENTRY(susword)

// TODO: implement suswintr
ENTRY(suswintr)

// TODO: implement subyte
ENTRY(subyte)

// TODO: implement fuswintr
ENTRY(fuswintr)

// TODO: implement kcopy
ENTRY(kcopy)

// TODO: implement setjmp
ENTRY(setjmp)

// TODO: implement longjmp
ENTRY(longjmp)

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
	addi 	sp, sp, -16
	sw	(sp+0), fp
	sw	(sp+4), ra
	sw	(sp+8), r11
	sw	(sp+12), r12
	ori	fp, r0, 16
	add	fp, sp, sp

	/* Save old context. */
	lw	r11, (r1+L_PCB)
	sw	(r11+PCB_SP), sp
	sw	(r11+PCB_FP), fp
	sw	(r11+PCB_RA), ra

	/* Switch to newlwp's stack. */
1:	lw	r12, (r2+L_PCB)
	lw	ra, (r12+PCB_RA)
	lw	fp, (r12+PCB_FP)
	lw	sp, (r12+PCB_SP)

	/* Set curlwp. */
	SET_CPUVAR(CURLWP,r12)

	/* Skip the rest if returning to a pinned LWP. */
	bne	r3, r0, 4f /* branch if r3 != 0 */

	/* Check if this process is using RAS (restartable atomic sequence). */
	/* TODO: check for RAS */
	lw	r12, (r2+L_PROC)
	lw	r12, (r12+P_RASLIST)
	bne	r12, r0, 5f
	
	/* Return to the new LWP, returning 'oldlwp' in r1. */
4:
	lw	r12, (sp+12)
	lw	r11, (sp+8)
	lw	ra, (sp+4)
	lw	fp, (sp+0)
	addi	sp, sp, 16
	ret

	/* Check for restartable atomic sequence (RAS). */
5:
	sw	(sp+0), r1
	sw	(sp+4), r2
	addi	sp, sp, 8
	lw	r2, (r2+(L_MD_REGS+MD_UTF+TF_REGS+R_PC))
	lw	r1, (sp+4)
	lw	r1, (r1+L_PROC)

	/* call ras_lookup(newlwp->l_proc, newlwp.md_regs.md_utf.tf_regs.r_pc); */
	calli	_C_LABEL(ras_lookup)

	/* save return value in tf_regs.r_pc */
	sw	(r2+R_PC), r1

	lw	r2, (sp+4)
	lw	r1, (sp+0)
	addi	sp, sp, -8
	bi	4b
