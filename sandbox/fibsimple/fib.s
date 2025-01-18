	.text
	.file	"LLVMDialectModule"
	.p2align	4, 0x90                         # -- Begin function closure_worker_1
	.type	.Lclosure_worker_1,@function
.Lclosure_worker_1:                     # @closure_worker_1
	.cfi_startproc
# %bb.0:
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movq	%rdx, (%rsp)                    # 8-byte Spill
	movl	%edi, %eax
	movq	(%rsp), %rdi                    # 8-byte Reload
	movl	%eax, 16(%rsp)                  # 4-byte Spill
	movl	%esi, 24(%rsp)                  # 4-byte Spill
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	callq	show@PLT
	movl	16(%rsp), %esi                  # 4-byte Reload
	movq	%rax, %rdi
	movq	%rdi, 32(%rsp)                  # 8-byte Spill
	callq	sigi_push_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	movl	$1, %esi
	callq	sigi_push_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 20(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	movl	20(%rsp), %eax                  # 4-byte Reload
	subl	%eax, %esi
	callq	sigi_push_i32@PLT
	movl	24(%rsp), %esi                  # 4-byte Reload
	movq	32(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	movl	$10, %esi
	callq	sigi_push_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 28(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	movl	28(%rsp), %eax                  # 4-byte Reload
	imull	%eax, %esi
	callq	sigi_push_i32@PLT
	movq	32(%rsp), %rdi                  # 8-byte Reload
	callq	fibloop@PLT
	addq	$40, %rsp
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
	movq	%rsi, %rdx
	movq	%rdi, %rax
	movl	24(%rax), %edi
	movl	28(%rax), %esi
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
	movq	%rdi, %rax
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
	.globl	apply                           # -- Begin function apply
	.p2align	4, 0x90
	.type	apply,@function
apply:                                  # @apply
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	%rdi, (%rsp)                    # 8-byte Spill
	callq	sigi_pop_closure@PLT
	movq	(%rsp), %rsi                    # 8-byte Reload
	movq	%rax, %rdi
	callq	*(%rax)
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end5:
	.size	apply, .Lfunc_end5-apply
	.cfi_endproc
                                        # -- End function
	.globl	show                            # -- Begin function show
	.p2align	4, 0x90
	.type	show,@function
show:                                   # @show
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	callq	sigi_builtin__pp@PLT
	movq	%rax, %rdi
	movq	%rdi, (%rsp)                    # 8-byte Spill
	callq	sigi_pop_i32@PLT
                                        # kill: def $ecx killed $eax
	movq	(%rsp), %rax                    # 8-byte Reload
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end6:
	.size	show, .Lfunc_end6-show
	.cfi_endproc
                                        # -- End function
	.globl	fibloop                         # -- Begin function fibloop
	.p2align	4, 0x90
	.type	fibloop,@function
fibloop:                                # @fibloop
	.cfi_startproc
# %bb.0:
	subq	$56, %rsp
	.cfi_def_cfa_offset 64
	movq	%rdi, 40(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 24(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	movl	%esi, 28(%rsp)                  # 4-byte Spill
	callq	sigi_push_i32@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	xorl	%esi, %esi
	callq	sigi_push_i32@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 20(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movl	20(%rsp), %ecx                  # 4-byte Reload
	movq	40(%rsp), %rdi                  # 8-byte Reload
	subl	%ecx, %eax
	sete	%al
	movzbl	%al, %esi
	callq	sigi_push_bool@PLT
                                        # implicit-def: $rdi
	callq	malloc@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movq	%rax, %rsi
	movq	$.Lclosure_drop_nothing, 16(%rsi)
	movl	$0, 8(%rsi)
	movq	$.Lclosure_wrapper_0, (%rsi)
	callq	sigi_push_closure@PLT
                                        # implicit-def: $rdi
	callq	malloc@PLT
	movl	24(%rsp), %edx                  # 4-byte Reload
	movl	28(%rsp), %ecx                  # 4-byte Reload
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movq	%rax, %rsi
	movl	%edx, 28(%rax)
	movl	%ecx, 24(%rax)
	movq	$.Lclosure_drop_nothing, 16(%rax)
	movl	$0, 8(%rax)
	movq	$.Lclosure_wrapper_1, (%rax)
	callq	sigi_push_closure@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_closure@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	callq	sigi_pop_closure@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	callq	sigi_pop_bool@PLT
	testb	$1, %al
	jne	.LBB7_1
	jmp	.LBB7_2
.LBB7_1:
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	jmp	.LBB7_3
.LBB7_2:
	movq	32(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	jmp	.LBB7_3
.LBB7_3:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%rax, (%rsp)                    # 8-byte Spill
# %bb.4:
	movq	40(%rsp), %rdi                  # 8-byte Reload
	movq	(%rsp), %rsi                    # 8-byte Reload
	callq	sigi_push_closure@PLT
	movq	40(%rsp), %rdi                  # 8-byte Reload
	callq	apply@PLT
	addq	$56, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end7:
	.size	fibloop, .Lfunc_end7-fibloop
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
	movq	%rdi, (%rsp)                    # 8-byte Spill
	movl	$10, %esi
	callq	sigi_push_i32@PLT
	movq	(%rsp), %rdi                    # 8-byte Reload
	movl	$1, %esi
	callq	sigi_push_i32@PLT
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	fibloop@PLT
	popq	%rcx
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
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	__main__@PLT
	movq	(%rsp), %rdi                    # 8-byte Reload
	callq	sigi_free_stack@PLT
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end9:
	.size	main, .Lfunc_end9-main
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
