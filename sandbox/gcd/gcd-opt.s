	.text
	.file	"LLVMDialectModule"
	.globl	gcd                             # -- Begin function gcd
	.p2align	4, 0x90
	.type	gcd,@function
gcd:                                    # @gcd
	.cfi_startproc
# %bb.0:
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movq	%rdi, 24(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	24(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 32(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movl	%eax, %ecx
	movl	32(%rsp), %eax                  # 4-byte Reload
	movl	%ecx, 36(%rsp)                  # 4-byte Spill
	cmpl	$0, %eax
	jne	.LBB0_2
# %bb.1:
	movq	24(%rsp), %rdi                  # 8-byte Reload
	movl	36(%rsp), %esi                  # 4-byte Reload
	callq	sigi_push_i32@PLT
	movq	24(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 16(%rsp)                  # 8-byte Spill
	jmp	.LBB0_3
.LBB0_2:
	movq	24(%rsp), %rdi                  # 8-byte Reload
	movl	32(%rsp), %esi                  # 4-byte Reload
	callq	sigi_push_i32@PLT
	movl	36(%rsp), %eax                  # 4-byte Reload
	movl	32(%rsp), %ecx                  # 4-byte Reload
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cltd
	idivl	%ecx
	movl	%edx, %esi
	callq	sigi_push_i32@PLT
	movq	24(%rsp), %rdi                  # 8-byte Reload
	callq	gcd@PLT
	movq	%rax, 16(%rsp)                  # 8-byte Spill
.LBB0_3:
	movq	16(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 8(%rsp)                   # 8-byte Spill
# %bb.4:
	movq	8(%rsp), %rax                   # 8-byte Reload
	addq	$40, %rsp
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
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	movl	$24, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	$8, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	callq	gcd@PLT
	movq	%rax, %rdi
	callq	sigi_builtin__pp@PLT
	movq	%rax, %rdi
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
                                        # kill: def $ecx killed $eax
	movq	16(%rsp), %rax                  # 8-byte Reload
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
	movq	(%rsp), %rdi                    # 8-byte Reload
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
	.section	".note.GNU-stack","",@progbits
