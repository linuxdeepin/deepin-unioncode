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

.global has_ymm_registers;
.hidden has_ymm_registers;
.type   has_ymm_registers, @function

.global syscall_no_intercept;
.type   syscall_no_intercept, @function

.text

has_ymm_registers:
	.cfi_startproc
	pushq       %rbx
	movq        $0x1, %rax
	cpuid
	movq        %rcx, %rax
	shrq        $28, %rax
	andq        $1, %rax
	popq        %rbx
	retq
	.cfi_endproc

.size   has_ymm_registers, .-has_ymm_registers

syscall_no_intercept:
	movq        %rdi, %rax  /* convert from linux ABI calling */
	movq        %rsi, %rdi  /* convention to syscall calling convention */
	movq        %rdx, %rsi
	movq        %rcx, %rdx
	movq        %r8, %r10
	movq        %r9, %r8
	movq        8(%rsp), %r9
	syscall
	ret

.size   syscall_no_intercept, .-syscall_no_intercept
