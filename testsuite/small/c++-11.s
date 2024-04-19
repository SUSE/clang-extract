	.file	"c++-11.cpp"
	.text
	.globl	_Z1fv
	.type	_Z1fv, @function
_Z1fv:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	_ZNSt4fposIiEC1Ev
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	_Z1fv, .-_Z1fv
	.section	.text._ZNSt4fposIiEC2Ev,"axG",@progbits,_ZNSt4fposIiEC5Ev,comdat
	.align 2
	.weak	_ZNSt4fposIiEC2Ev
	.type	_ZNSt4fposIiEC2Ev, @function
_ZNSt4fposIiEC2Ev:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	$0, (%rax)
	movq	-8(%rbp), %rax
	movl	$0, 8(%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	_ZNSt4fposIiEC2Ev, .-_ZNSt4fposIiEC2Ev
	.weak	_ZNSt4fposIiEC1Ev
	.set	_ZNSt4fposIiEC1Ev,_ZNSt4fposIiEC2Ev
	.ident	"GCC: (SUSE Linux) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
