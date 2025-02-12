	.text
	.file	"LLVMDialectModule"
	.globl	gcd                             # -- Begin function gcd
	.p2align	4, 0x90
	.type	gcd,@function
gcd:                                    # @gcd
	.cfi_startproc
# %bb.0:
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movl	%edi, 16(%rsp)                  # 4-byte Spill
	movl	%esi, 20(%rsp)                  # 4-byte Spill
	cmpl	$0, %edi
	jne	.LBB0_2
# %bb.1:
	movl	20(%rsp), %eax                  # 4-byte Reload
	movl	%eax, 12(%rsp)                  # 4-byte Spill
	jmp	.LBB0_3
.LBB0_2:
	movl	16(%rsp), %esi                  # 4-byte Reload
	movl	20(%rsp), %eax                  # 4-byte Reload
	xorl	%edx, %edx
	divl	%esi
	movl	%edx, %edi
	callq	gcd@PLT
	movl	%eax, 12(%rsp)                  # 4-byte Spill
.LBB0_3:
	movl	12(%rsp), %eax                  # 4-byte Reload
	movl	%eax, 8(%rsp)                   # 4-byte Spill
# %bb.4:
	movl	8(%rsp), %eax                   # 4-byte Reload
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	gcd, .Lfunc_end0-gcd
	.cfi_endproc
                                        # -- End function
	.globl	__main__                        # -- Begin function __main__
	.p2align	4, 0x90
	.type	__main__,@function
__main__:                               # @__main__
	.cfi_startproc
# %bb.0:
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movl	$8, %edi
	movl	$24, %esi
	callq	gcd@PLT
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	callq	sigi_builtin__pp@PLT
	movq	%rax, %rdi
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rcx                  # 8-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	__main__, .Lfunc_end1-__main__
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$128, %edi
	callq	malloc@PLT
	movq	%rax, %rdi
	movq	%rdi, (%rsp)                    # 8-byte Spill
	callq	sigi_init_stack@PLT
	movq	(%rsp), %rcx                    # 8-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	__main__@PLT
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	sigi_free_stack@PLT
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.type	sigi.global.stack,@object       # @sigi.global.stack
	.bss
	.weak	sigi.global.stack
	.p2align	3, 0x0
sigi.global.stack:
	.zero	8
	.size	sigi.global.stack, 8

	.section	".note.GNU-stack","",@progbits
