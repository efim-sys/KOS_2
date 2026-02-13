	.file	"test.c"
	.text
	.align	4
	.global	triple
	.type	triple, @function
triple:
	entry	sp, 32
	addx2	a2, a2, a2
	retw.n
	.size	triple, .-triple
	.section	.text.startup,"ax",@progbits
	.literal_position
	.literal .LC0, var
	.literal .LC1, quad
	.align	4
	.global	main
	.type	main, @function
main:
	entry	sp, 32
	call8	do_magic
	l32r	a8, .LC0
	l32i	a10, a8, 0
	l32r	a8, .LC1
	l32i	a8, a8, 0
	callx8	a8
	addi	a10, a10, 81
	float.s	f0, a10, 0
	rfr	a2, f0
	retw.n
	.size	main, .-main
	.global	quad
	.data
	.align	4
	.type	quad, @object
	.size	quad, 4
quad:
	.word	5768
	.global	var
	.section	.bss
	.align	4
	.type	var, @object
	.size	var, 4
var:
	.zero	4
	.ident	"GCC: (crosstool-NG esp-14.2.0_20241119) 14.2.0"
