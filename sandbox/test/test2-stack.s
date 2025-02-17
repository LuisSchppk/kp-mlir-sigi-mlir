	.text
	.file	"LLVMDialectModule"
	.globl	pow_loop                        # -- Begin function pow_loop
	.p2align	4, 0x90
	.type	pow_loop,@function
pow_loop:                               # @pow_loop
	.cfi_startproc
# %bb.0:
	subq	$88, %rsp
	.cfi_def_cfa_offset 96
	movl	%esi, %eax
	movl	%edi, %esi
	movl	%eax, 60(%rsp)                  # 4-byte Spill
	movl	%edx, 64(%rsp)                  # 4-byte Spill
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	%rdi, 72(%rsp)                  # 8-byte Spill
	callq	sigi_push_i32@PLT
	movl	60(%rsp), %esi                  # 4-byte Reload
	movq	72(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movl	64(%rsp), %esi                  # 4-byte Reload
	movq	72(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	72(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	72(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 68(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	72(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 80(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movl	%eax, %ecx
	movl	80(%rsp), %eax                  # 4-byte Reload
	movl	%ecx, 84(%rsp)                  # 4-byte Spill
	cmpl	$0, %eax
	jg	.LBB0_2
# %bb.1:
	movq	72(%rsp), %rax                  # 8-byte Reload
	movl	68(%rsp), %ecx                  # 4-byte Reload
	movb	$1, %dl
	movl	%ecx, 40(%rsp)                  # 4-byte Spill
	movb	%dl, 47(%rsp)                   # 1-byte Spill
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	jmp	.LBB0_3
.LBB0_2:
	movl	68(%rsp), %edi                  # 4-byte Reload
	callq	sigi_builtin__pp_i32@PLT
	movl	84(%rsp), %esi                  # 4-byte Reload
	movq	72(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movl	80(%rsp), %esi                  # 4-byte Reload
	movq	72(%rsp), %rdi                  # 8-byte Reload
	subl	$1, %esi
	callq	sigi_push_i32@PLT
	movl	84(%rsp), %esi                  # 4-byte Reload
	movl	68(%rsp), %eax                  # 4-byte Reload
	movq	72(%rsp), %rdi                  # 8-byte Reload
	imull	%eax, %esi
	callq	sigi_push_i32@PLT
	movq	72(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	72(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 36(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	72(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 32(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	72(%rsp), %rcx                  # 8-byte Reload
	movl	32(%rsp), %esi                  # 4-byte Reload
	movl	36(%rsp), %edx                  # 4-byte Reload
	movl	%eax, %edi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	pow_loop@PLT
	movl	%eax, %ecx
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movl	%ecx, 40(%rsp)                  # 4-byte Spill
	movb	%dl, 47(%rsp)                   # 1-byte Spill
	movq	%rax, 48(%rsp)                  # 8-byte Spill
.LBB0_3:
	movl	40(%rsp), %eax                  # 4-byte Reload
	movb	47(%rsp), %cl                   # 1-byte Reload
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	%rdx, 16(%rsp)                  # 8-byte Spill
	movb	%cl, 27(%rsp)                   # 1-byte Spill
	movl	%eax, 28(%rsp)                  # 4-byte Spill
# %bb.4:
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	28(%rsp), %esi                  # 4-byte Reload
	callq	sigi_push_i32@PLT
	movb	27(%rsp), %al                   # 1-byte Reload
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movzbl	%al, %esi
	callq	sigi_push_bool@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_bool@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movb	%al, 15(%rsp)                   # 1-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rsi                  # 8-byte Reload
	movb	15(%rsp), %dl                   # 1-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rcx
	movq	%rsi, (%rcx)
	addq	$88, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	pow_loop, .Lfunc_end0-pow_loop
	.cfi_endproc
                                        # -- End function
	.globl	__main__                        # -- Begin function __main__
	.p2align	4, 0x90
	.type	__main__,@function
__main__:                               # @__main__
	.cfi_startproc
# %bb.0:
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	movl	$2, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	$6, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	$1, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	%eax, 24(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	%eax, 20(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	8(%rsp), %rcx                   # 8-byte Reload
	movl	20(%rsp), %esi                  # 4-byte Reload
	movl	24(%rsp), %edx                  # 4-byte Reload
	movl	%eax, %edi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	pow_loop@PLT
	movl	%eax, 28(%rsp)                  # 4-byte Spill
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movzbl	%dl, %edi
	callq	sigi_builtin__pp_i1@PLT
	movl	28(%rsp), %edi                  # 4-byte Reload
	callq	sigi_builtin__pp_i32@PLT
	movq	32(%rsp), %rcx                  # 8-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	addq	$40, %rsp
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
	xorl	%eax, %eax
                                        # kill: def $al killed $al killed $eax
	popq	%rcx
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
