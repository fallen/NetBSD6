/*	$NetBSD: $	*/

#define	ELF32_MACHDEP_ENDIANNESS	ELFDATA2MSB
#define	ELF32_MACHDEP_ID_CASES						\
		case EM_LATTICEMICO32:						\
			break;

#define	ELF64_MACHDEP_ENDIANNESS	XXX	/* break compilation */
#define	ELF64_MACHDEP_ID_CASES						\
		/* no 64-bit ELF machine types supported */

#define	ELF32_MACHDEP_ID		EM_LATTICEMICO32

#define ARCH_ELFSIZE		32	/* MD native binary size */

/* iLM32 relocations */
#define	R_LM32_NONE		0
#define	R_LM32_8		1
#define	R_LM32_16		2
#define	R_LM32_32		3
#define	R_LM32_HI16		4
#define	R_LM32_LO16		5
#define	R_LM32_GPREL16		6
#define	R_LM32_CALL		7
#define	R_LM32_BRANCH		8
#define R_LM32_GNU_VTINHERIT 	9
#define R_LM32_GNU_VTENTRY 	10
#define R_LM32_16_GOT 		11
#define R_LM32_GOTOFF_HI16 	12
#define R_LM32_GOTOFF_LO16 	13
#define R_LM32_COPY 		14
#define R_LM32_GLOB_DAT 	15
#define R_LM32_JMP_SLOT 	16
#define R_LM32_RELATIVE 	17


#define	R_TYPE(name)	__CONCAT(R_LM32_,name)
