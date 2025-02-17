	.text
	.file	"LLVMDialectModule"
	.p2align	4, 0x90                         # -- Begin function closure_worker_1
	.type	.Lclosure_worker_1,@function
.Lclosure_worker_1:                     # @closure_worker_1
	.cfi_startproc
# %bb.0:
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 12(%rsp)                  # 4-byte Spill
	callq	sigi_pop_bool@PLT
	movzbl	%al, %edi
	callq	sigi_builtin__pp_i1@PLT
	movl	12(%rsp), %esi                  # 4-byte Reload
	movq	16(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	16(%rsp), %rax                  # 8-byte Reload
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	.Lclosure_worker_1, .Lfunc_end0-.Lclosure_worker_1
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90                         # -- Begin function closure_wrapper_1
	.type	.Lclosure_wrapper_1,@function
.Lclosure_wrapper_1:                    # @closure_wrapper_1
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	%rsi, (%rsp)                    # 8-byte Spill
	movq	%rdi, %rax
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	.Lclosure_worker_1
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	.Lclosure_wrapper_1, .Lfunc_end1-.Lclosure_wrapper_1
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90                         # -- Begin function closure_drop_nothing
	.type	.Lclosure_drop_nothing,@function
.Lclosure_drop_nothing:                 # @closure_drop_nothing
	.cfi_startproc
# %bb.0:
	retq
.Lfunc_end2:
	.size	.Lclosure_drop_nothing, .Lfunc_end2-.Lclosure_drop_nothing
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90                         # -- Begin function closure_worker_0
	.type	.Lclosure_worker_0,@function
.Lclosure_worker_0:                     # @closure_worker_0
	.cfi_startproc
# %bb.0:
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	movl	%esi, 12(%rsp)                  # 4-byte Spill
	subl	$1, %esi
	callq	sigi_push_i32@PLT
	movl	12(%rsp), %eax                  # 4-byte Reload
	movq	16(%rsp), %rdi                  # 8-byte Reload
	cmpl	$0, %eax
	setg	%al
	movzbl	%al, %esi
	callq	sigi_push_bool@PLT
	movq	16(%rsp), %rax                  # 8-byte Reload
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end3:
	.size	.Lclosure_worker_0, .Lfunc_end3-.Lclosure_worker_0
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90                         # -- Begin function closure_wrapper_0
	.type	.Lclosure_wrapper_0,@function
.Lclosure_wrapper_0:                    # @closure_wrapper_0
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	%rsi, (%rsp)                    # 8-byte Spill
	movq	%rdi, %rax
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	.Lclosure_worker_0
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end4:
	.size	.Lclosure_wrapper_0, .Lfunc_end4-.Lclosure_wrapper_0
	.cfi_endproc
                                        # -- End function
	.globl	while                           # -- Begin function while
	.p2align	4, 0x90
	.type	while,@function
while:                                  # @while
	.cfi_startproc
# %bb.0:
	subq	$88, %rsp
	.cfi_def_cfa_offset 96
	movq	%rcx, 64(%rsp)                  # 8-byte Spill
	movq	%rdx, 48(%rsp)                  # 8-byte Spill
	movl	%esi, 36(%rsp)                  # 4-byte Spill
	movb	%dil, %al
	movq	sigi.global.stack@GOTPCREL(%rip), %rcx
	movq	(%rcx), %rdi
	movq	%rdi, 40(%rsp)                  # 8-byte Spill
	movzbl	%al, %esi
	callq	sigi_push_bool@PLT
	movl	36(%rsp), %esi                  # 4-byte Reload
	movq	40(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	40(%rsp), %rsi                  # 8-byte Reload
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	%rdx, %rdi
	callq	*(%rdx)
	movq	%rax, %rdi
	movq	%rdi, 56(%rsp)                  # 8-byte Spill
	callq	sigi_pop_bool@PLT
	movq	56(%rsp), %rsi                  # 8-byte Reload
	movq	64(%rsp), %rcx                  # 8-byte Reload
	movb	%al, 79(%rsp)                   # 1-byte Spill
	movq	%rcx, %rdi
	callq	*(%rcx)
	movq	%rax, %rcx
	movb	79(%rsp), %al                   # 1-byte Reload
	movq	%rcx, 80(%rsp)                  # 8-byte Spill
	testb	$1, %al
	jne	.LBB5_1
	jmp	.LBB5_2
.LBB5_1:
	movq	80(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	80(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 12(%rsp)                  # 4-byte Spill
	callq	sigi_pop_bool@PLT
	movq	80(%rsp), %r8                   # 8-byte Reload
	movl	12(%rsp), %esi                  # 4-byte Reload
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	64(%rsp), %rcx                  # 8-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rdi
	movq	%r8, (%rdi)
	movzbl	%al, %edi
	callq	while@PLT
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_push_i32@PLT
	movq	16(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 24(%rsp)                  # 8-byte Spill
	jmp	.LBB5_3
.LBB5_2:
	movq	80(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 24(%rsp)                  # 8-byte Spill
	jmp	.LBB5_3
.LBB5_3:
	movq	24(%rsp), %rax                  # 8-byte Reload
	movq	%rax, (%rsp)                    # 8-byte Spill
# %bb.4:
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	(%rsp), %rdx                    # 8-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rcx
	movq	%rdx, (%rcx)
	addq	$88, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end5:
	.size	while, .Lfunc_end5-while
	.cfi_endproc
                                        # -- End function
	.globl	print_stack_and_pop             # -- Begin function print_stack_and_pop
	.p2align	4, 0x90
	.type	print_stack_and_pop,@function
print_stack_and_pop:                    # @print_stack_and_pop
	.cfi_startproc
# %bb.0:
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movl	%esi, 4(%rsp)                   # 4-byte Spill
	movb	%dil, %al
	movb	%al, 23(%rsp)                   # 1-byte Spill
                                        # implicit-def: $rdi
	callq	malloc@PLT
	movq	%rax, %rcx
	movq	%rcx, 8(%rsp)                   # 8-byte Spill
	movq	$.Lclosure_drop_nothing, 16(%rax)
	movl	$0, 8(%rax)
	movq	$.Lclosure_wrapper_0, (%rax)
                                        # implicit-def: $rdi
	callq	malloc@PLT
	movl	4(%rsp), %esi                   # 4-byte Reload
	movq	8(%rsp), %rdx                   # 8-byte Reload
	movq	%rax, %rdi
	movb	23(%rsp), %al                   # 1-byte Reload
	movq	%rdi, %rcx
	movq	$.Lclosure_drop_nothing, 16(%rdi)
	movl	$0, 8(%rdi)
	movq	$.Lclosure_wrapper_1, (%rdi)
	movzbl	%al, %edi
	callq	while@PLT
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end6:
	.size	print_stack_and_pop, .Lfunc_end6-print_stack_and_pop
	.cfi_endproc
                                        # -- End function
	.globl	pow_loop                        # -- Begin function pow_loop
	.p2align	4, 0x90
	.type	pow_loop,@function
pow_loop:                               # @pow_loop
	.cfi_startproc
# %bb.0:
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movl	%edi, 28(%rsp)                  # 4-byte Spill
	movl	%esi, 32(%rsp)                  # 4-byte Spill
	movl	%edx, 36(%rsp)                  # 4-byte Spill
	cmpl	$0, %esi
	jg	.LBB7_2
# %bb.1:
	movl	36(%rsp), %eax                  # 4-byte Reload
	movb	$1, %dl
	movl	%eax, 20(%rsp)                  # 4-byte Spill
	movb	%dl, 27(%rsp)                   # 1-byte Spill
	jmp	.LBB7_3
.LBB7_2:
	movl	36(%rsp), %edi                  # 4-byte Reload
	callq	sigi_builtin__pp_i32@PLT
	movl	32(%rsp), %esi                  # 4-byte Reload
	movl	36(%rsp), %eax                  # 4-byte Reload
	movl	28(%rsp), %edi                  # 4-byte Reload
	subl	$1, %esi
	movl	%edi, %edx
	imull	%eax, %edx
	callq	pow_loop@PLT
	movl	%eax, 20(%rsp)                  # 4-byte Spill
	movb	%dl, 27(%rsp)                   # 1-byte Spill
.LBB7_3:
	movl	20(%rsp), %eax                  # 4-byte Reload
	movb	27(%rsp), %cl                   # 1-byte Reload
	movb	%cl, 15(%rsp)                   # 1-byte Spill
	movl	%eax, 16(%rsp)                  # 4-byte Spill
# %bb.4:
	movb	15(%rsp), %dl                   # 1-byte Reload
	movl	16(%rsp), %eax                  # 4-byte Reload
	addq	$40, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end7:
	.size	pow_loop, .Lfunc_end7-pow_loop
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
	movl	$2, %edi
	movl	%edi, 12(%rsp)                  # 4-byte Spill
	movl	$6, %esi
	movl	$1, %edx
	callq	pow_loop@PLT
	movl	%eax, %esi
	movb	%dl, 11(%rsp)                   # 1-byte Spill
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rax, 24(%rsp)                  # 8-byte Spill
	movq	(%rax), %rdi
	movq	%rdi, (%rsp)                    # 8-byte Spill
	callq	sigi_push_i32@PLT
	movq	(%rsp), %rcx                    # 8-byte Reload
	movb	11(%rsp), %dl                   # 1-byte Reload
	movl	12(%rsp), %esi                  # 4-byte Reload
	movq	24(%rsp), %rax                  # 8-byte Reload
	movq	%rcx, (%rax)
	movzbl	%dl, %edi
	callq	print_stack_and_pop@PLT
	movl	%eax, %ecx
	movq	24(%rsp), %rax                  # 8-byte Reload
	movl	%ecx, 36(%rsp)                  # 4-byte Spill
	movq	(%rax), %rdi
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rsi                  # 8-byte Reload
	movq	24(%rsp), %rcx                  # 8-byte Reload
	movl	36(%rsp), %edx                  # 4-byte Reload
	movq	%rsi, (%rcx)
	addq	$40, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end8:
	.size	__main__, .Lfunc_end8-__main__
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
.Lfunc_end9:
	.size	main, .Lfunc_end9-main
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
