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

#include "easylogging++.h"
#include "WaitStatus.h"

#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

WaitStatus::Type WaitStatus::type() const {
  if (exit_code() >= 0) {
    return EXIT;
  }
  if (fatal_sig() > 0) {
    return FATAL_SIGNAL;
  }
  if (stop_sig() > 0) {
    return SIGNAL_STOP;
  }
  if (group_stop() > 0) {
    return GROUP_STOP;
  }
  if (is_syscall()) {
    return SYSCALL_STOP;
  }
  if (ptrace_event() > 0) {
    return PTRACE_EVENT;
  }
  LOG(FATAL) << "Status " << HEX(status) << " not understood";
  return EXIT;
}

int WaitStatus::exit_code() const {
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int WaitStatus::fatal_sig() const {
  return WIFSIGNALED(status) ? WTERMSIG(status) : 0;
}

int WaitStatus::stop_sig() const {
  if (!WIFSTOPPED(status) || ((status >> 16) & 0xff)) {
    return 0;
  }
  int sig = WSTOPSIG(status);
  if (sig == (SIGTRAP | 0x80)) {
    return 0;
  }
  sig &= ~0x80;
  return sig ? sig : SIGSTOP;
}

int WaitStatus::group_stop() const {
  if (!WIFSTOPPED(status) || ((status >> 16) & 0xff) != PTRACE_EVENT_STOP) {
    return 0;
  }
  int sig = WSTOPSIG(status);
  sig &= ~0x80;
  return sig ? sig : SIGSTOP;
}

bool WaitStatus::is_syscall() const {
  if (!WIFSTOPPED(status) || ptrace_event()) {
    return 0;
  }
  return WSTOPSIG(status) == (SIGTRAP | 0x80);
}

int WaitStatus::ptrace_event() const {
  int event = (status >> 16) & 0xff;
  return event == PTRACE_EVENT_STOP ? 0 : event;
}

int WaitStatus::ptrace_signal() const {
  return WIFSTOPPED(status) ? (WSTOPSIG(status) & 0x7f) : 0;
}
