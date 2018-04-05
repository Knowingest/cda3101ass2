	.text
	ori $s0,$0,25
	ori $s1,$0,22
	andi $t0,$s0,22
	lw $t1,24($0)
	lw $t2,28($0)
	halt
	.data
Data1:	.word	12
Data2:	.word	25
Data3:	.word	19
