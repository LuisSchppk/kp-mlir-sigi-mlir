	.text
	.file	"LLVMDialectModule"
	.globl	simpleSigi                      # -- Begin function simpleSigi
	.p2align	4, 0x90
	.type	simpleSigi,@function
simpleSigi:                             # @simpleSigi
	.cfi_startproc
# %bb.0:
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 4(%rsp)                   # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	callq	sigi_push_i32@PLT
	movl	4(%rsp), %esi                   # 4-byte Reload
	movq	16(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_push_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 8(%rsp)                   # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	movl	8(%rsp), %eax                   # 4-byte Reload
	imull	%eax, %esi
	callq	sigi_push_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	$1, %esi
	callq	sigi_push_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, 12(%rsp)                  # 4-byte Spill
	callq	sigi_pop_i32@PLT
	movq	16(%rsp), %rdi                  # 8-byte Reload
	movl	%eax, %esi
	movl	12(%rsp), %eax                  # 4-byte Reload
	addl	%eax, %esi
	callq	sigi_push_i32@PLT
	movq	16(%rsp), %rax                  # 8-byte Reload
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	simpleSigi, .Lfunc_end0-simpleSigi
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
