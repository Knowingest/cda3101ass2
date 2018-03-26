	.text
	ori	$s0,$0,24
	lw	$t1,0($s0)
	bne	$t5,$t6,LOOP
	sub	$t2,$t0,$t1
	lw	$t2,4($s0)
	noop
	add	$t3,$t1,$t2
LOOP:	sw	$t3,8($s0)
	andi	$s1,$s0,15
	sll	$t0,$t1,5
	halt
	.data
Data1:	.word	12
Data2:	.word	25
Data3:	.word	19