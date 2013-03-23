#ifndef __LM32_REGISTERS_H

struct registers {
	int	tf_r1;
	int	tf_r2;
	int	tf_r3;
	int	tf_r4;
	int	tf_r5;
	int	tf_r6;
	int	tf_r7;
	int	tf_r8;
	int	tf_r9;
	int	tf_r10;
	int	tf_r11;
	int	tf_r12;
	int	tf_r13;
	int	tf_r14;
	int	tf_r15;
	int	tf_r16;
	int	tf_r17;
	int	tf_r18;
	int	tf_r19;
	int	tf_r20;
	int	tf_r21;
	int	tf_r22;
	int	tf_r23;
	int	tf_r24;
	int	tf_r25;
	int 	tf_gp;
	int 	tf_fp;
	int 	tf_sp;
	int 	tf_ra;
	int 	tf_ea;
	int 	tf_ba;
};



#endif /* __LM32_REGISTERS_H */
