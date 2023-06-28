// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
/* For constants ORIG_EAX (32bit) OR ORIG_RAX(64bit) etc */
#include <linux/unistd.h>
#include <linux/capability.h>
#include <linux/magic.h>

#include "utils.h"

using namespace std;

bool has_effective_caps()
{
#if 0
  //TODO: SYS_capget not work!
  struct NativeArch::cap_header header = {.version =
                                              _LINUX_CAPABILITY_VERSION_3,
                                          .pid = 0 };
  struct NativeArch::cap_data data[_LINUX_CAPABILITY_U32S_3];
  if (syscall(SYS_capget, &header, data) != 0) {
    assert(0 && "FAILED to read capabilities");
  }
  for (int i = 0; i < _LINUX_CAPABILITY_U32S_3; ++i) {
    if ((data[i].effective & (uint32_t)caps) != (uint32_t)caps) {
      return false;
    }
    caps >>= 32;
  }
#endif

  return true;
}

bool ensure_dir(const string& dir, const char* dir_type, mode_t mode)
{
  string d = dir;
  while (!d.empty() && d[d.length() - 1] == '/') {
    d = d.substr(0, d.length() - 1);
  }

  struct stat st;
  if (0 > stat(d.c_str(), &st)) {
    if (errno != ENOENT) {
      return false;
    }

    size_t last_slash = d.find_last_of('/');
    if (last_slash == string::npos || last_slash == 0) {
      return false;
    }
    ensure_dir(d.substr(0, last_slash), dir_type, mode);

    // Allow for a race condition where someone else creates the directory
    if (0 > mkdir(d.c_str(), mode) && errno != EEXIST) {
      return false;
    }
    if (0 > stat(d.c_str(), &st)) {
      return false;
    }
  }

  if (!(S_IFDIR & st.st_mode)) {
    return false;
  }
  if (access(d.c_str(), W_OK)) {
    return false;
  }

  return true;
}

const char* tmp_dir()
{
  const char* dir = getenv("RR_TMPDIR");
  if (dir) {
    if (ensure_dir(string(dir), "temporary file directory (RR_TMPDIR)", S_IRWXU))
        return dir;
  }
  dir = getenv("TMPDIR");
  if (dir) {
    if (ensure_dir(string(dir), "temporary file directory (TMPDIR)", S_IRWXU))
        return dir;
  }
  // Don't try to create "/tmp", that probably won't work well.
  if (access("/tmp", W_OK)) {
    return nullptr;
  }
  return "/tmp";
}

#define CASE(_id)   \
  case _id:         \
    return #_id;

const char* ptrace_cmd_name(int request)
{
    switch (request) {
    CASE(PTRACE_TRACEME);
    CASE(PTRACE_PEEKTEXT);
    CASE(PTRACE_PEEKDATA);
    CASE(PTRACE_PEEKUSER);
    CASE(PTRACE_POKETEXT);
    CASE(PTRACE_POKEDATA);
    CASE(PTRACE_POKEUSER);
    CASE(PTRACE_CONT);
    CASE(PTRACE_KILL);
    CASE(PTRACE_SINGLESTEP);
    CASE(PTRACE_ATTACH);
    CASE(PTRACE_DETACH);

#if !defined(__aarch64__)
    CASE(PTRACE_GETREGS);
    CASE(PTRACE_SETREGS);
    CASE(PTRACE_GETFPREGS);
    CASE(PTRACE_SETFPREGS);
    CASE(PTRACE_GETFPXREGS);
    CASE(PTRACE_SETFPXREGS);
#endif

    CASE(PTRACE_SYSCALL);
    CASE(PTRACE_SETOPTIONS);
    CASE(PTRACE_GETEVENTMSG);
    CASE(PTRACE_GETSIGINFO);
    CASE(PTRACE_SETSIGINFO);
    CASE(PTRACE_GETREGSET);
    CASE(PTRACE_SETREGSET);
    CASE(PTRACE_SEIZE);
    CASE(PTRACE_INTERRUPT);
    CASE(PTRACE_LISTEN);
    CASE(PTRACE_PEEKSIGINFO);
#ifndef KYLIN_MIPS64
    CASE(PTRACE_GETSIGMASK);
    CASE(PTRACE_SETSIGMASK);
    CASE(PTRACE_SECCOMP_GET_FILTER);
#endif

    default: {
        static char buf[100];
        sprintf(buf, "(0x%x)", request);
        return (buf);
        }
    }
}

