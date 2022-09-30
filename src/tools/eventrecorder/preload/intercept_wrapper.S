/*
 * Copyright 2016-2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * intercept_wrapper.s -- see asm_wrapper.md
 */

/* the function in this file */
.global intercept_wrapper
.type intercept_wrapper, @function

/* the C function in intercept.c */
.global intercept_routine
.hidden intercept_routine
.type intercept_routine, @function

/* the other C function in intercept.c, called right after cloning a thread */
.global intercept_routine_post_clone
.hidden intercept_routine_post_clone
.type intercept_routine_post_clone, @function

/* The boolean indicating whether YMM registers must saved */
.global intercept_routine_must_save_ymm
.hidden intercept_routine_must_save_ymm

.text

/*
 * Local stack layout:
 *
 * 0x448(%rsp)  -- return address, to the generated asm wrapper
 * Arguments recieved on stack:
 * 0x450(%rsp)  -- original value of rsp
 * 0x458(%rsp)  -- pointer to a struct patch_desc instance
 * Locals on stack:
 * 0xe8(%rsp) - 0x168(%rsp) -- saved GPRs
 * 0x200(%rsp) - 0x400(%rsp) -- saved SIMD registers
 *
 * A pointer to these saved register is passed to intercept_routine, so the
 * layout of `struct context` must match this part of the stack layout.
 *
 * Other arguments:
 * %rcx  -- which C function to call
 */
intercept_wrapper:
	.cfi_startproc

	/*
	 * Stack size used locally: 0x448 bytes.
	 *
	 * This size assumes the stack pointer was correctly aligned before
	 * executing the call instruction calling this function. The return
	 * address pushed to the stack uses 8 bytes. This gives the equation:
	 *
	 * new_rsp = original_rsp - 8 - 0x448 == original_rsp - 0x450
	 * The number 0x450 is a multiple of 16, so the stack is still correctly
	 * aligned. It is very easy to forget about this when making changes to this
	 * code.
	 */
	subq        $0x448, %rsp
	.cfi_def_cfa_offset 0x0

	/* Save all GPRs on the stack */
	movq        %rax, 0x160 (%rsp)
	.cfi_offset 0, 0x160
	movq        %rdx, 0x158 (%rsp)
	.cfi_offset 1, 0x158
	/* rcx is already clobbered, no reason to save it */
	movq        %rbx, 0x150 (%rsp)
	.cfi_offset 3, 0x150
	movq        %rsi, 0x148 (%rsp)
	.cfi_offset 4, 0x148
	movq        %rdi, 0x140 (%rsp)
	.cfi_offset 5, 0x140
	movq        %rbp, 0x138 (%rsp)
	.cfi_offset 6, 0x138
	movq        0x450 (%rsp), %r11 /* fetch original value of rsp */
	movq        %r11, 0x130 (%rsp)
	.cfi_offset 7, 0x130
	movq        %r8, 0x128 (%rsp)
	.cfi_offset 8, 0x128
	movq        %r9, 0x120 (%rsp)
	.cfi_offset 9, 0x120
	movq        %r10, 0x118 (%rsp)
	.cfi_offset 10, -0x118
	/* r11 is already clobbered, no reason to save it */
	movq        %r12, 0x110 (%rsp)
	.cfi_offset 12, 0x110
	movq        %r13, 0x108 (%rsp)
	.cfi_offset 13, 0x108
	movq        %r14, 0x100 (%rsp)
	.cfi_offset 14, 0x100
	movq        %r15, 0xf8 (%rsp)
	.cfi_offset 15, 0xf8
	movq        0x458 (%rsp), %r11 /* fetch pointer to patch_desc */
	movq        %r11, 0xe8 (%rsp)
	movq        (%r11), %r11 /* fetch original value of rip */
	movq        %r11, 0xf0 (%rsp)
	.cfi_offset 16, 0xf0

	movb        intercept_routine_must_save_ymm (%rip), %al
	test        %al, %al
	jz          0f

	/*
	 * Save the YMM registers.
	 * Use vmovups. Must not use vmovaps, since 32 byte alignment is not
	 * guaranteed.
	 * One could just align the stack for this, but that would need
	 * more explanation in comments in other places about why overaligned
	 * stack is needed.
	 */
	vmovups     %ymm0, 0x3c0 (%rsp)
	vmovups     %ymm1, 0x380 (%rsp)
	vmovups     %ymm2, 0x340 (%rsp)
	vmovups     %ymm3, 0x300 (%rsp)
	vmovups     %ymm4, 0x2c0 (%rsp)
	vmovups     %ymm5, 0x280 (%rsp)
	vmovups     %ymm6, 0x240 (%rsp)
	vmovups     %ymm7, 0x200 (%rsp)
	jmp         1f

0:
	/* Save the XMM registers. */
	movaps      %xmm0, 0x3c0 (%rsp)
	movaps      %xmm1, 0x380 (%rsp)
	movaps      %xmm2, 0x340 (%rsp)
	movaps      %xmm3, 0x300 (%rsp)
	movaps      %xmm4, 0x2c0 (%rsp)
	movaps      %xmm5, 0x280 (%rsp)
	movaps      %xmm6, 0x240 (%rsp)
	movaps      %xmm7, 0x200 (%rsp)

1:
	/* argument passed to intercept_routine */
	leaq        0xe8 (%rsp), %rdi

	cmp         $0x1, %rcx /* which function should be called? */
	je          0f
	call        intercept_routine
	jmp         1f
0:	call        intercept_routine_post_clone
1:
	/*
	 * At this point, the return value of the C
	 * function (a struct wrapper_ret instance) is in rax, rdx.
	 *
	 * This function doesn't use these values for anything, just
	 * forwards them to the higher level wrapper function, generated
	 * from the template.
	 */

	movq        %rdx, %r11
	/*
	 * At this point, the return values of this asm function
	 * are in rax, r11.
	 *
	 * Restore the other registers, and return.
	 */

	movb        intercept_routine_must_save_ymm (%rip), %dl
	test        %dl, %dl
	jz          0f

	vmovups     0x3c0 (%rsp), %ymm0
	vmovups     0x380 (%rsp), %ymm1
	vmovups     0x340 (%rsp), %ymm2
	vmovups     0x300 (%rsp), %ymm3
	vmovups     0x2c0 (%rsp), %ymm4
	vmovups     0x280 (%rsp), %ymm5
	vmovups     0x240 (%rsp), %ymm6
	vmovups     0x200 (%rsp), %ymm7
	jmp         1f

0:
	movaps      0x3c0 (%rsp), %xmm0
	movaps      0x380 (%rsp), %xmm1
	movaps      0x340 (%rsp), %xmm2
	movaps      0x300 (%rsp), %xmm3
	movaps      0x2c0 (%rsp), %xmm4
	movaps      0x280 (%rsp), %xmm5
	movaps      0x240 (%rsp), %xmm6
	movaps      0x200 (%rsp), %xmm7

1:
	movq        0x158 (%rsp), %rdx
	movq        0x150 (%rsp), %rbx
	movq        0x148 (%rsp), %rsi
	movq        0x140 (%rsp), %rdi
	movq        0x138 (%rsp), %rbp
	movq        0x128 (%rsp), %r8
	movq        0x120 (%rsp), %r9
	movq        0x118 (%rsp), %r10
	movq        0x110 (%rsp), %r12
	movq        0x108 (%rsp), %r13
	movq        0x100 (%rsp), %r14
	movq        0xf8 (%rsp), %r15

	addq        $0x448, %rsp

	retq
	.cfi_endproc
