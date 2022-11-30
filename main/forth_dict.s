.set latest,0

.macro NEXT
	l32i a0,a2,0
	addi a2,a2,4
	jx a0
.endm

.macro DOCOL
	addi a5,a5,-4
	s32i a2,a5,0
	addi a2,a0,20
	l32i a0,a0,16
	jx a0
	.align 4
.endm

.macro cw name,len
	.text
	.align 4
	.global link_\name
	link_\name:
		.int latest
		.int \name + 3 
		.set latest, link_\name
		.int \len
		.asciz "\name"
.endm

.macro dw name,len
	.text
	.align 4
	.global link_\name
	link_\name:
		.int latest
		.int \name
		.set latest, link_\name
		.int \len
		.asciz "\name"
	.align 4
	.global \name
	\name:
	DOCOL
.endm

.text

dw "Hello",5
	.int 1,2,3,4
