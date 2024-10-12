   asm("\
	.macro PUSHD\n\
	addi	a4, a4, -4\n\
	s32i.n	a3, a4, 0\n\
	.endm\n\
	.macro NEXT\n\
	l32i.n	a7, a2, 0\n\
	addi.n	a2, a2, 4\n\
	jx	a7\n\
	.endm\n\
	.macro POPD\n\
	l32i.n	a3, a4, 0\n\
	addi.n	a4, a4, 4\n\
	.endm\n\
   ");
