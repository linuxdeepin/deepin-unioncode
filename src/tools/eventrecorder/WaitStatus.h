/*
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -*- Mode: C++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: nil; -*- */

#ifndef RR_WAIT_STATUS_H_
#define RR_WAIT_STATUS_H_

#include <signal.h>

#include <memory>
#include <vector>

#ifndef PTRACE_EVENT_NONE
#define PTRACE_EVENT_NONE 0
#endif
#ifndef PTRACE_EVENT_STOP
#define PTRACE_EVENT_STOP 128
#endif

#ifndef PTRACE_O_TRACESECCOMP
#define PTRACE_O_TRACESECCOMP 0x00000080
#define PTRACE_EVENT_SECCOMP_OBSOLETE 8 // ubuntu 12.04
#define PTRACE_EVENT_SECCOMP 7          // ubuntu 12.10 and future kernels
#endif

class WaitStatus {
public:
  explicit WaitStatus(int status = 0) : status(status) {}

  enum Type {
    // Task exited normally.
    EXIT,
    // Task exited due to fatal signal.
    FATAL_SIGNAL,
    // Task is in a signal-delivery-stop.
    SIGNAL_STOP,
    // Task is in a group-stop. (See ptrace man page.)
    // You must use PTRACE_SEIZE to generate PTRACE_EVENT_STOPs, or these
    // will be treated as STOP_SIGNAL.
    GROUP_STOP,
    // Task is in a syscall-stop triggered by PTRACE_SYSCALL
    // and PTRACE_O_TRACESYSGOOD.
    SYSCALL_STOP,
    // Task is in a PTRACE_EVENT stop, except for PTRACE_EVENT_STOP
    // which is treated as GROUP_STOP.
    PTRACE_EVENT
  };

  Type type() const;

  // Exit code if type() == EXIT, otherwise -1.
  int exit_code() const;
  // Fatal signal if type() == FATAL_SIGNAL, otherwise zero.
  int fatal_sig() const;
  // Stop signal if type() == STOP_SIGNAL, otherwise zero. A zero signal
  // (rare but observed via PTRACE_INTERRUPT) is converted to SIGSTOP.
  int stop_sig() const;
  // Stop signal if type() == GROUP_STOP, otherwise zero. A zero signal
  // (rare but observed via PTRACE_INTERRUPT) is converted to SIGSTOP.
  int group_stop() const;
  bool is_syscall() const;
  // ptrace event if type() == PTRACE_EVENT, otherwise zero.
  int ptrace_event() const;

  // For exit_code() and fatal_sig(), returns 0. For all other types
  // returns the signal involved.
  int ptrace_signal() const;

  int get() const { return status; }

private:
  int status;
};

#endif /* RR_WAIT_STATUS_H_ */
