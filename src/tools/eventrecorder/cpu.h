/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuchanghui<liuchanghui@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _CPU_H
#define _CPU_H

#if defined(__i386__) || defined(__x86_64__)
#include <sys/user.h>
#include <sys/reg.h>
typedef struct user_regs_struct USER_REGS;
typedef struct user_fpregs_struct USER_FPREGS;

#if defined(__i386__)
#define SYSCALL_NUMBER_INDEX    ORIG_EAX
#define SYSCALL_RESULT_INDEX    EAX
#define STACK_POINTER(regs)     regs.esp

#elif defined(__x86_64__)
#define SYSCALL_NUMBER_INDEX    ORIG_RAX
#define SYSCALL_RESULT_INDEX    RAX
#define STACK_POINTER(regs)     regs.rsp
#endif

#elif defined(__arm__) || defined(__aarch64__)
typedef struct tagUSER_REGS {
    unsigned long x0;
    unsigned long x1;
    unsigned long x2;
    unsigned long x3;
    unsigned long x4;
    unsigned long x5;
    unsigned long x6;
    unsigned long x7;
    unsigned long x8;
    unsigned long x9;
    unsigned long x10;
    unsigned long x11;
    unsigned long x12;
    unsigned long x13;
    unsigned long x14;
    unsigned long x15;
    unsigned long x16;
    unsigned long x17;
    unsigned long x18;
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long x29;
    unsigned long ra; //x30;
    unsigned long sp;
    unsigned long pc;
    unsigned long pstate;
    /*unsigned long orig_x0;
    unsigned long syscallno;
    unsigned long orig_addr_limit;
    unsigned long unused;	// maintain 16 byte alignment
    */
}USER_REGS;

typedef struct tagUSER_FPREGS {
    long fpregs[66];
}USER_FPREGS;

#define SYSCALL_NUMBER_INDEX    8 /*$x8 register*/
#define SYSCALL_RESULT_INDEX    0 /*$x0 register*/
#define STACK_POINTER(regs)     regs.sp

#elif defined(__mips__) || defined(__mips64)

#define DT_MIPS_RLD_MAP_REL  0x70000035

typedef struct tagUSER_REGS {
    long zero;//$0    /* wired zero */
    long at;//$1      /* assembler temp  - uppercase because of ".set at" */
    long v0;//$2      /* return value */
    long v1;//$3
    long a0;//$4      /* argument registers */
    long a1;//$5
    long a2;//$6
    long a3;//$7

#if _MIPS_SIM != _ABIO32
    long a4;//$8
    long a5;//$9
    long a6;//$10
    long a7;//$11
    long t0;//$12
    long t1;//$13
    long t2;//$14
    long t3;//$15
#else /* if _MIPS_SIM == _ABIO32 */
    long t0;//$8      /* caller saved */
    long t1;//$9
    long t2;//$10
    long t3;//$11
    long t4;//$12
    long t5;//$13
    long t6;//$14
    long t7;//$15
#endif /* _MIPS_SIM == _ABIO32 */
    long s0;//$16     /* callee saved */
    long s1;//$17
    long s2;//$18
    long s3;//$19
    long s4;//$20
    long s5;//$21
    long s6;//$22
    long s7;//$23
    long t8;//$24     /* caller saved */
    long t9;//$25
    long k0;//$26     /* kernel scratch */
    long k1;//$27
    long gp;//$28     /* global pointer */
    long sp;//$29     /* stack pointer */
    long fp;//$30     /* frame pointer */
    long ra;//$31     /* return address */

    /* Saved special registers. */
    long lo;
    long hi;
    long pc; //cp0_epc;
    long cp0_badvaddr;
    long cp0_status;
    long cp0_cause;
}USER_REGS;

typedef struct tagUSER_FPREGS {
    long fpregs[33];
}USER_FPREGS;

#define SYSCALL_NUMBER_INDEX    2 /*$v0 register*/
#define SYSCALL_RESULT_INDEX    2 /*$v0 register*/
#define STACK_POINTER(regs)     regs.sp

#elif defined(__sw_64)
// refer to /usr/include/asm/reg.h
// arch/sw_64/kernel/ptrace.c

typedef struct tagUSER_REGS {
    unsigned long v0; //r0;
    unsigned long t0; //r1;
    unsigned long t1; //r2;
    unsigned long t2; //r3;
    unsigned long t3; //r4;
    unsigned long t4; //r5;
    unsigned long t5; //r6;
    unsigned long t6; //r7;
    unsigned long t7; //r8;
    unsigned long s0; //r9;
    unsigned long s1;//r10;
    unsigned long s2;//r11;
    unsigned long s3;//r12;
    unsigned long s4;//r13;
    unsigned long s5;//r14;
    unsigned long s6;//r15;

    unsigned long a3; //r19;
    unsigned long a4; //r20;
    unsigned long a5; //r21;
    unsigned long t8; //r22;
    unsigned long t9; //r23;
    unsigned long t10;//r24;
    unsigned long t11;//r25;
    unsigned long ra; //r26;
    unsigned long t12;//r27;
    unsigned long r28;

    unsigned long sp; //r30
    unsigned long ps; //r27
    unsigned long pc; //
    unsigned long gp; //r29
    unsigned long a0; //r16;
    unsigned long a1; //r17;
    unsigned long a2; //r18;
}USER_REGS;

typedef struct tagUSER_FPREGS {
    long fpregs[32];
}USER_FPREGS;

#define SYSCALL_NUMBER_INDEX    0 /*$v0 register*/
#define SYSCALL_RESULT_INDEX    0 /*$v0 register*/
#define STACK_POINTER(regs)     regs.sp

#else
#error need to define new SYSCALL_RESULT_INDEX
#endif

#endif // end #ifndef _CPU_H
