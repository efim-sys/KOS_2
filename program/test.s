	.file	"test.c"
	.text
	.section	.text.startup,"ax",@progbits
	.literal_position
	.literal .LC0, zero
	.literal .LC2, 1084227584
	.align	4
	.global	main
	.type	main, @function
main:
	entry	sp, 48
	l32r	a8, .LC0
	wfr	f1, a2
	l32i	a8, a8, 0
	s32i.n	a2, sp, 0
	float.s	f0, a8, 0
	l32r	a8, .LC2
	wfr	f2, a8
	madd.s	f0, f1, f2
	rfr	a2, f0
	retw.n
	.size	main, .-main
	.global	zero
	.section	.bss
	.align	4
	.type	zero, @object
	.size	zero, 4
zero:
	.zero	4
	.ident	"GCC: (crosstool-NG esp-14.2.0_20241119) 14.2.0"
