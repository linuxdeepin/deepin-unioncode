// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _SYSCALL_FILTER_H
#define _SYSCALL_FILTER_H

#define SUPPORTED_PERSONALITIES  1

#define TRACE_FILE	001	/* Trace file-related syscalls. */
#define TRACE_IPC	002	/* Trace IPC-related syscalls. */
#define TRACE_NETWORK	004	/* Trace network-related syscalls. */
#define TRACE_PROCESS	010	/* Trace process-related syscalls. */
#define TRACE_SIGNAL	020	/* Trace signal-related syscalls. */
#define TRACE_DESC	040	/* Trace file descriptor-related syscalls. */
#define TRACE_MEMORY	0100	/* Trace memory mapping-related syscalls. */
#define SYSCALL_NEVER_FAILS	0200	/* Syscall is always successful. */
#define STACKTRACE_INVALIDATE_CACHE 0400  /* Trigger proc/maps cache updating */
#define STACKTRACE_CAPTURE_ON_ENTER 01000 /* Capture stacktrace on "entering" stage */
#define TRACE_INDIRECT_SUBCALL	02000	/* Syscall is an indirect socket/ipc subcall. */

/* Define these shorthand notations to simplify the syscallent files. */
#define TD TRACE_DESC
#define TF TRACE_FILE
#define TI TRACE_IPC
#define TN TRACE_NETWORK
#define TP TRACE_PROCESS
#define TS TRACE_SIGNAL
#define TM TRACE_MEMORY
#define NF SYSCALL_NEVER_FAILS
#define MA MAX_ARGS
#define SI STACKTRACE_INVALIDATE_CACHE
#define SE STACKTRACE_CAPTURE_ON_ENTER

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SEN(syscall_name) 0

typedef struct sysent {
	unsigned int nargs;
	int	sys_flags;
	int	sen;
	const char *sys_name;
} struct_sysent;


#endif /*#ifndef _SYSCALL_FILTER_H*/
