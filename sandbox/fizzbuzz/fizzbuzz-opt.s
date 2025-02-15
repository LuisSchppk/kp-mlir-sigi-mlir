	.text
	.file	"LLVMDialectModule"
	.globl	fizzbuzz                        # -- Begin function fizzbuzz
	.p2align	4, 0x90
	.type	fizzbuzz,@function
fizzbuzz:                               # @fizzbuzz
	.cfi_startproc
# %bb.0:
	subq	$104, %rsp
	.cfi_def_cfa_offset 112
	movl	%edi, 84(%rsp)                  # 4-byte Spill
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	%rdi, 88(%rsp)                  # 8-byte Spill
	callq	sigi_push_i32@PLT
	movl	84(%rsp), %esi                  # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 96(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movl	96(%rsp), %ecx                  # 4-byte Reload
	movl	%eax, 100(%rsp)                 # 4-byte Spill
	cmpl	%ecx, %eax
	jg	.LBB0_14
# %bb.1:
	movl	100(%rsp), %eax                 # 4-byte Reload
	andl	$1, %eax
	cmpl	$0, %eax
	jne	.LBB0_7
# %bb.2:
	movl	100(%rsp), %eax                 # 4-byte Reload
	movl	$5, %ecx
	xorl	%edx, %edx
	divl	%ecx
	cmpl	$0, %edx
	jne	.LBB0_4
# %bb.3:
	movl	$4294967293, %edi               # imm = 0xFFFFFFFD
	callq	sigi_builtin__pp_i32@PLT
	movl	100(%rsp), %esi                 # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	addl	$1, %esi
	callq	sigi_push_i32@PLT
	movl	96(%rsp), %esi                  # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 68(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rcx                  # 8-byte Reload
	movl	68(%rsp), %edi                  # 4-byte Reload
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	fizzbuzz@PLT
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, 72(%rsp)                  # 8-byte Spill
	jmp	.LBB0_5
.LBB0_4:
	movl	$4294967295, %edi               # imm = 0xFFFFFFFF
	callq	sigi_builtin__pp_i32@PLT
	movl	100(%rsp), %esi                 # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	addl	$1, %esi
	callq	sigi_push_i32@PLT
	movl	96(%rsp), %esi                  # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 64(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rcx                  # 8-byte Reload
	movl	64(%rsp), %edi                  # 4-byte Reload
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	fizzbuzz@PLT
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, 72(%rsp)                  # 8-byte Spill
.LBB0_5:
	movq	72(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 56(%rsp)                  # 8-byte Spill
# %bb.6:
	movq	56(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	jmp	.LBB0_12
.LBB0_7:
	movl	100(%rsp), %eax                 # 4-byte Reload
	movl	$5, %ecx
	xorl	%edx, %edx
	divl	%ecx
	cmpl	$0, %edx
	jne	.LBB0_9
# %bb.8:
	movl	$4294967294, %edi               # imm = 0xFFFFFFFE
	callq	sigi_builtin__pp_i32@PLT
	movl	100(%rsp), %esi                 # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	addl	$1, %esi
	callq	sigi_push_i32@PLT
	movl	96(%rsp), %esi                  # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 36(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rcx                  # 8-byte Reload
	movl	36(%rsp), %edi                  # 4-byte Reload
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	fizzbuzz@PLT
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	jmp	.LBB0_10
.LBB0_9:
	movl	100(%rsp), %edi                 # 4-byte Reload
	callq	sigi_builtin__pp_i32@PLT
	movl	100(%rsp), %esi                 # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	addl	$1, %esi
	callq	sigi_push_i32@PLT
	movl	96(%rsp), %esi                  # 4-byte Reload
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 32(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	88(%rsp), %rcx                  # 8-byte Reload
	movl	32(%rsp), %edi                  # 4-byte Reload
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	fizzbuzz@PLT
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, 40(%rsp)                  # 8-byte Spill
.LBB0_10:
	movq	40(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 24(%rsp)                  # 8-byte Spill
# %bb.11:
	movq	24(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	jmp	.LBB0_12
.LBB0_12:
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 16(%rsp)                  # 8-byte Spill
# %bb.13:
	movq	16(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	jmp	.LBB0_15
.LBB0_14:
	movq	88(%rsp), %rax                  # 8-byte Reload
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	jmp	.LBB0_15
.LBB0_15:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%rax, (%rsp)                    # 8-byte Spill
# %bb.16:
	movq	(%rsp), %rcx                    # 8-byte Reload
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	addq	$104, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	fizzbuzz, .Lfunc_end0-fizzbuzz
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
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	xorl	%esi, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	$100, %esi
	callq	sigi_push_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	8(%rsp), %rdi                   # 8-byte Reload
	movl	%eax, 20(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	8(%rsp), %rcx                   # 8-byte Reload
	movl	20(%rsp), %edi                  # 4-byte Reload
	movl	%eax, %esi
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	%rcx, (%rax)
	callq	fizzbuzz@PLT
	movq	sigi.global.stack@GOTPCREL(%rip), %rax
	movq	(%rax), %rcx
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
