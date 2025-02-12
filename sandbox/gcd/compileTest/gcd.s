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
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$8, %edi
	movl	$24, %esi
	callq	gcd@PLT
	movl	%eax, %edi
	callq	sigi_builtin__pp_i32@PLT
	popq	%rax
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
	callq	__main__@PLT
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
