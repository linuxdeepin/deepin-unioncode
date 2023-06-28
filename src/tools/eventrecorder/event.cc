// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#if defined(__aarch64__)
#include "./aarch64/syscall.h"
#else
#include <sys/syscall.h>
#endif

#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <linux/capability.h>
#include <linux/magic.h>

#include "event_man.h"
#include "shared_mem_dump.h"
#include "syscall_name.h"

#define CASE(_id)   \
  case _id:         \
    return #_id;

const char* signal_name(int sig)
{
  /* strsignal() would be nice to use here, but it provides TMI. */
  if (32 <= sig && sig <= 64) {
    static char buf[100];
    snprintf(buf, sizeof(buf) - 1, "SIGRT%d", sig);
    return buf;
  }

  switch (sig) {
    CASE(SIGHUP);
    CASE(SIGINT);
    CASE(SIGQUIT);
    CASE(SIGILL);
    CASE(SIGTRAP);
    CASE(SIGABRT); /*CASE(SIGIOT);*/
    CASE(SIGBUS);
    CASE(SIGFPE);
    CASE(SIGKILL);
    CASE(SIGUSR1);
    CASE(SIGSEGV);
    CASE(SIGUSR2);
    CASE(SIGPIPE);
    CASE(SIGALRM);
    CASE(SIGTERM);
#if defined(__i386__) || defined(__x86_64__)
    CASE(SIGSTKFLT); /*CASE(SIGCLD);*/
#endif
    CASE(SIGCHLD);
    CASE(SIGCONT);
    CASE(SIGSTOP);
    CASE(SIGTSTP);
    CASE(SIGTTIN);
    CASE(SIGTTOU);
    CASE(SIGURG);
    CASE(SIGXCPU);
    CASE(SIGXFSZ);
    CASE(SIGVTALRM);
    CASE(SIGPROF);
    CASE(SIGWINCH); /*CASE(SIGPOLL);*/
    CASE(SIGIO);
    CASE(SIGPWR);
    CASE(SIGSYS);
    /* Special-case this so we don't need to sprintf in this common case.
     * This case is common because we often pass signal_name(sig) to assertions
     * when sig is 0.
     */
    case 0:
      return "signal(0)";
    default: {
      static char buf[100];
      sprintf(buf, "signal(%d)", sig);
      return (buf);
    }
  }
}

const char* errno_name(int err) {
  switch (err) {
    case 0:
      return "SUCCESS";
      CASE(EPERM);
      CASE(ENOENT);
      CASE(ESRCH);
      CASE(EINTR);
      CASE(EIO);
      CASE(ENXIO);
      CASE(E2BIG);
      CASE(ENOEXEC);
      CASE(EBADF);
      CASE(ECHILD);
      CASE(EAGAIN);
      CASE(ENOMEM);
      CASE(EACCES);
      CASE(EFAULT);
      CASE(ENOTBLK);
      CASE(EBUSY);
      CASE(EEXIST);
      CASE(EXDEV);
      CASE(ENODEV);
      CASE(ENOTDIR);
      CASE(EISDIR);
      CASE(EINVAL);
      CASE(ENFILE);
      CASE(EMFILE);
      CASE(ENOTTY);
      CASE(ETXTBSY);
      CASE(EFBIG);
      CASE(ENOSPC);
      CASE(ESPIPE);
      CASE(EROFS);
      CASE(EMLINK);
      CASE(EPIPE);
      CASE(EDOM);
      CASE(ERANGE);
      CASE(EDEADLK);
      CASE(ENAMETOOLONG);
      CASE(ENOLCK);
      CASE(ENOSYS);
      CASE(ENOTEMPTY);
      CASE(ELOOP);
      CASE(ENOMSG);
      CASE(EIDRM);
      CASE(ECHRNG);
      CASE(EL2NSYNC);
      CASE(EL3HLT);
      CASE(EL3RST);
      CASE(ELNRNG);
      CASE(EUNATCH);
      CASE(ENOCSI);
      CASE(EL2HLT);
      CASE(EBADE);
      CASE(EBADR);
      CASE(EXFULL);
      CASE(ENOANO);
      CASE(EBADRQC);
      CASE(EBADSLT);
      CASE(EBFONT);
      CASE(ENOSTR);
      CASE(ENODATA);
      CASE(ETIME);
      CASE(ENOSR);
      CASE(ENONET);
      CASE(ENOPKG);
      CASE(EREMOTE);
      CASE(ENOLINK);
      CASE(EADV);
      CASE(ESRMNT);
      CASE(ECOMM);
      CASE(EPROTO);
      CASE(EMULTIHOP);
      CASE(EDOTDOT);
      CASE(EBADMSG);
      CASE(EOVERFLOW);
      CASE(ENOTUNIQ);
      CASE(EBADFD);
      CASE(EREMCHG);
      CASE(ELIBACC);
      CASE(ELIBBAD);
      CASE(ELIBSCN);
      CASE(ELIBMAX);
      CASE(ELIBEXEC);
      CASE(EILSEQ);
      CASE(ERESTART);
      CASE(ESTRPIPE);
      CASE(EUSERS);
      CASE(ENOTSOCK);
      CASE(EDESTADDRREQ);
      CASE(EMSGSIZE);
      CASE(EPROTOTYPE);
      CASE(ENOPROTOOPT);
      CASE(EPROTONOSUPPORT);
      CASE(ESOCKTNOSUPPORT);
      CASE(EOPNOTSUPP);
      CASE(EPFNOSUPPORT);
      CASE(EAFNOSUPPORT);
      CASE(EADDRINUSE);
      CASE(EADDRNOTAVAIL);
      CASE(ENETDOWN);
      CASE(ENETUNREACH);
      CASE(ENETRESET);
      CASE(ECONNABORTED);
      CASE(ECONNRESET);
      CASE(ENOBUFS);
      CASE(EISCONN);
      CASE(ENOTCONN);
      CASE(ESHUTDOWN);
      CASE(ETOOMANYREFS);
      CASE(ETIMEDOUT);
      CASE(ECONNREFUSED);
      CASE(EHOSTDOWN);
      CASE(EHOSTUNREACH);
      CASE(EALREADY);
      CASE(EINPROGRESS);
      CASE(ESTALE);
      CASE(EUCLEAN);
      CASE(ENOTNAM);
      CASE(ENAVAIL);
      CASE(EISNAM);
      CASE(EREMOTEIO);
      CASE(EDQUOT);
      CASE(ENOMEDIUM);
      CASE(EMEDIUMTYPE);
      CASE(ECANCELED);
      CASE(ENOKEY);
      CASE(EKEYEXPIRED);
      CASE(EKEYREVOKED);
      CASE(EKEYREJECTED);
      CASE(EOWNERDEAD);
      CASE(ENOTRECOVERABLE);
      CASE(ERFKILL);
      CASE(EHWPOISON);
    default: {
      static char buf[100];
      sprintf(buf, "errno(%d)", err);
      return (buf);
    }
  }
}

const char* sicode_name(int code, int sig) {
  switch (code) {
    CASE(SI_USER);
    CASE(SI_KERNEL);
    CASE(SI_QUEUE);
    CASE(SI_TIMER);
    CASE(SI_MESGQ);
    CASE(SI_ASYNCIO);
    CASE(SI_SIGIO);
    CASE(SI_TKILL);
    CASE(SI_ASYNCNL);
  }

  switch (sig) {
    case SIGSEGV:
      switch (code) {
        CASE(SEGV_MAPERR);
        CASE(SEGV_ACCERR);
      }
      break;
    case SIGTRAP:
      switch (code) {
        CASE(TRAP_BRKPT);
        CASE(TRAP_TRACE);
      }
      break;
    case SIGILL:
      switch (code) {
        CASE(ILL_ILLOPC);
        CASE(ILL_ILLOPN);
        CASE(ILL_ILLADR);
        CASE(ILL_ILLTRP);
        CASE(ILL_PRVOPC);
        CASE(ILL_PRVREG);
        CASE(ILL_COPROC);
        CASE(ILL_BADSTK);
      }
      break;
    case SIGFPE:
      switch (code) {
        CASE(FPE_INTDIV);
        CASE(FPE_INTOVF);
        CASE(FPE_FLTDIV);
        CASE(FPE_FLTOVF);
        CASE(FPE_FLTUND);
        CASE(FPE_FLTRES);
        CASE(FPE_FLTINV);
        CASE(FPE_FLTSUB);
      }
      break;
    case SIGBUS:
      switch (code) {
        CASE(BUS_ADRALN);
        CASE(BUS_ADRERR);
        CASE(BUS_OBJERR);
        CASE(BUS_MCEERR_AR);
        CASE(BUS_MCEERR_AO);
      }
      break;
    case SIGCHLD:
      switch (code) {
        CASE(CLD_EXITED);
        CASE(CLD_KILLED);
        CASE(CLD_DUMPED);
        CASE(CLD_TRAPPED);
        CASE(CLD_STOPPED);
        CASE(CLD_CONTINUED);
      }
      break;
    case SIGPOLL:
      switch (code) {
        CASE(POLL_IN);
        CASE(POLL_OUT);
        CASE(POLL_MSG);
        CASE(POLL_ERR);
        CASE(POLL_PRI);
        CASE(POLL_HUP);
      }
      break;
  }

  static char buf[100];
  sprintf(buf, "sicode(%d)", code);
  return (buf);
}

// see /usr/include/x86_64-linux-gnu/asm/unistd_64.h
const char* syscall_name(long call)
{
  switch(call) {
    CASE(SYS_read);
    CASE(SYS_write);
    CASE(SYS_open);
    CASE(SYS_close);
    CASE(SYS_stat);
    CASE(SYS_fstat);
    CASE(SYS_lstat);
    CASE(SYS_poll);
    CASE(SYS_lseek);
    CASE(SYS_mmap);
    CASE(SYS_mprotect);
    CASE(SYS_munmap);
    CASE(SYS_brk);
    CASE(SYS_rt_sigaction);
    CASE(SYS_rt_sigprocmask);
    CASE(SYS_rt_sigreturn);
    CASE(SYS_ioctl);
    CASE(SYS_pread64);
    CASE(SYS_pwrite64);
    CASE(SYS_readv);
    CASE(SYS_writev);
    CASE(SYS_access);
    CASE(SYS_pipe);
    CASE(SYS_sched_yield);
    CASE(SYS_mremap);
    CASE(SYS_msync);
    CASE(SYS_mincore);
    CASE(SYS_madvise);
    CASE(SYS_shmget);
    CASE(SYS_shmctl);
    CASE(SYS_dup);
    CASE(SYS_dup2);
    CASE(SYS_nanosleep);
    CASE(SYS_getitimer);
    CASE(SYS_setitimer);
    CASE(SYS_sendfile);
    CASE(SYS_socket);
    CASE(SYS_connect);
    CASE(SYS_accept);
    CASE(SYS_sendto);
    CASE(SYS_recvfrom);
    CASE(SYS_sendmsg);
    CASE(SYS_recvmsg);
    CASE(SYS_shutdown);
    CASE(SYS_bind);
    CASE(SYS_listen);
    CASE(SYS_getsockname);
    CASE(SYS_getpeername);
    CASE(SYS_socketpair);
    CASE(SYS_setsockopt);
    CASE(SYS_getsockopt);
    CASE(SYS_clone);
    CASE(SYS_fork);
    CASE(SYS_execve);
    CASE(SYS_exit);
    CASE(SYS_wait4);
    CASE(SYS_kill);
    CASE(SYS_uname);
    CASE(SYS_semget);
    CASE(SYS_semop);
    CASE(SYS_semctl);
    CASE(SYS_shmdt);
    CASE(SYS_msgget);
    CASE(SYS_msgsnd);
    CASE(SYS_msgrcv);
    CASE(SYS_msgctl);
    CASE(SYS_fcntl);
    CASE(SYS_flock);
    CASE(SYS_fsync);
    CASE(SYS_fdatasync);
    CASE(SYS_truncate);
    CASE(SYS_ftruncate);
    CASE(SYS_getdents);
    CASE(SYS_getcwd);
    CASE(SYS_chdir);
    CASE(SYS_fchdir);
    CASE(SYS_rename);
    CASE(SYS_mkdir);
    CASE(SYS_rmdir);
    CASE(SYS_link);
    CASE(SYS_unlink);
    CASE(SYS_symlink);
    CASE(SYS_readlink);
    CASE(SYS_chmod);
    CASE(SYS_fchmod);
    CASE(SYS_chown);
    CASE(SYS_fchown);
    CASE(SYS_lchown);
    CASE(SYS_umask);
    CASE(SYS_gettimeofday);
    CASE(SYS_getrlimit);
    CASE(SYS_getrusage);
    CASE(SYS_sysinfo);
    CASE(SYS_times);
    CASE(SYS_ptrace);
    CASE(SYS_syslog);
    CASE(SYS_setuid);
    CASE(SYS_setgid);
    CASE(SYS_setpgid);
    CASE(SYS_getpgrp);
    CASE(SYS_setsid);
    CASE(SYS_setreuid);
    CASE(SYS_setregid);
    CASE(SYS_getgroups);
    CASE(SYS_setgroups);
    CASE(SYS_setresuid);
    CASE(SYS_getresuid);
    CASE(SYS_setresgid);
    CASE(SYS_getresgid);
    CASE(SYS_getpgid);
    CASE(SYS_setfsuid);
    CASE(SYS_setfsgid);
    CASE(SYS_getsid);
    CASE(SYS_capget);
    CASE(SYS_capset);
    CASE(SYS_rt_sigpending);
    CASE(SYS_rt_sigtimedwait);
    CASE(SYS_rt_sigqueueinfo);
    CASE(SYS_rt_sigsuspend);
    CASE(SYS_sigaltstack);
    CASE(SYS_mknod);
    CASE(SYS_personality);
    CASE(SYS_ustat);
    CASE(SYS_statfs);
    CASE(SYS_fstatfs);
    CASE(SYS_getpriority);
    CASE(SYS_setpriority);
    CASE(SYS_sched_setparam);
    CASE(SYS_sched_getparam);
    CASE(SYS_sched_setscheduler);
    CASE(SYS_sched_getscheduler);
    CASE(SYS_sched_get_priority_max);
    CASE(SYS_sched_get_priority_min);
    CASE(SYS_sched_rr_get_interval);
    CASE(SYS_mlock);
    CASE(SYS_munlock);
    CASE(SYS_mlockall);
    CASE(SYS_munlockall);
    CASE(SYS_vhangup);
    CASE(SYS_pivot_root);
    CASE(SYS__sysctl);
    CASE(SYS_prctl);
    CASE(SYS_adjtimex);
    CASE(SYS_setrlimit);
    CASE(SYS_chroot);
    CASE(SYS_sync);
    CASE(SYS_acct);
    CASE(SYS_settimeofday);
    CASE(SYS_mount);
    CASE(SYS_swapon);
    CASE(SYS_swapoff);
    CASE(SYS_reboot);
    CASE(SYS_sethostname);
    CASE(SYS_setdomainname);
    CASE(SYS_init_module);
    CASE(SYS_delete_module);
    CASE(SYS_quotactl);
    CASE(SYS_nfsservctl);
    CASE(SYS_gettid);
    CASE(SYS_readahead);
    CASE(SYS_setxattr);
    CASE(SYS_lsetxattr);
    CASE(SYS_fsetxattr);
    CASE(SYS_getxattr);
    CASE(SYS_lgetxattr);
    CASE(SYS_fgetxattr);
    CASE(SYS_listxattr);
    CASE(SYS_llistxattr);
    CASE(SYS_flistxattr);
    CASE(SYS_removexattr);
    CASE(SYS_lremovexattr);
    CASE(SYS_fremovexattr);
    CASE(SYS_tkill);
    CASE(SYS_futex);
    CASE(SYS_sched_setaffinity);
    CASE(SYS_sched_getaffinity);
    CASE(SYS_io_setup);
    CASE(SYS_io_destroy);
    CASE(SYS_io_getevents);
    CASE(SYS_io_submit);
    CASE(SYS_io_cancel);
    CASE(SYS_lookup_dcookie);
    CASE(SYS_epoll_create);
    CASE(SYS_remap_file_pages);
    CASE(SYS_getdents64);
    CASE(SYS_set_tid_address);
    CASE(SYS_restart_syscall);
    CASE(SYS_semtimedop);
    CASE(SYS_fadvise64);
    CASE(SYS_timer_create);
    CASE(SYS_timer_settime);
    CASE(SYS_timer_gettime);
    CASE(SYS_timer_getoverrun);
    CASE(SYS_timer_delete);
    CASE(SYS_clock_settime);
    CASE(SYS_clock_gettime);
    CASE(SYS_clock_getres);
    CASE(SYS_clock_nanosleep);
    CASE(SYS_exit_group);
    CASE(SYS_epoll_wait);
    CASE(SYS_epoll_ctl);
    CASE(SYS_tgkill);
    CASE(SYS_mbind);
    CASE(SYS_set_mempolicy);
    CASE(SYS_get_mempolicy);
    CASE(SYS_mq_open);
    CASE(SYS_mq_unlink);
    CASE(SYS_mq_timedsend);
    CASE(SYS_mq_timedreceive);
    CASE(SYS_mq_notify);
    CASE(SYS_mq_getsetattr);
    CASE(SYS_kexec_load);
    CASE(SYS_waitid);
    CASE(SYS_add_key);
    CASE(SYS_request_key);
    CASE(SYS_keyctl);
    CASE(SYS_ioprio_set);
    CASE(SYS_ioprio_get);
    CASE(SYS_inotify_init);
    CASE(SYS_inotify_add_watch);
    CASE(SYS_inotify_rm_watch);
    CASE(SYS_migrate_pages);
    CASE(SYS_openat);
    CASE(SYS_mkdirat);
    CASE(SYS_mknodat);
    CASE(SYS_fchownat);
    CASE(SYS_futimesat);
    CASE(SYS_unlinkat);
    CASE(SYS_renameat);
    CASE(SYS_linkat);
    CASE(SYS_symlinkat);
    CASE(SYS_readlinkat);
    CASE(SYS_fchmodat);
    CASE(SYS_faccessat);
    CASE(SYS_pselect6);
    CASE(SYS_ppoll);
    CASE(SYS_unshare);
    CASE(SYS_set_robust_list);
    CASE(SYS_get_robust_list);
    CASE(SYS_splice);
    CASE(SYS_tee);
    CASE(SYS_sync_file_range);
    CASE(SYS_vmsplice);
    CASE(SYS_move_pages);
    CASE(SYS_epoll_pwait);
    CASE(SYS_signalfd);
    CASE(SYS_timerfd_create);
    CASE(SYS_eventfd);
    CASE(SYS_fallocate);
    CASE(SYS_timerfd_settime);
    CASE(SYS_timerfd_gettime);
    CASE(SYS_accept4);
    CASE(SYS_signalfd4);
    CASE(SYS_eventfd2);
    CASE(SYS_epoll_create1);
    CASE(SYS_dup3);
    CASE(SYS_pipe2);
    CASE(SYS_inotify_init1);
    CASE(SYS_preadv);
    CASE(SYS_pwritev);
    CASE(SYS_rt_tgsigqueueinfo);
    CASE(SYS_perf_event_open);
    CASE(SYS_recvmmsg);
    CASE(SYS_fanotify_init);
    CASE(SYS_fanotify_mark);
    CASE(SYS_prlimit64);
    CASE(SYS_name_to_handle_at);
    CASE(SYS_open_by_handle_at);
    CASE(SYS_clock_adjtime);
    CASE(SYS_syncfs);
    CASE(SYS_sendmmsg);
    CASE(SYS_setns);
    CASE(SYS_getcpu);
    CASE(SYS_process_vm_readv);
    CASE(SYS_process_vm_writev);
    CASE(SYS_kcmp);
    CASE(SYS_finit_module);

#ifndef KYLIN_MIPS64
    CASE(SYS_sched_setattr);
    CASE(SYS_sched_getattr);
    CASE(SYS_renameat2);
    CASE(SYS_getrandom);
    CASE(SYS_memfd_create);
    CASE(SYS_execveat);
#endif

    CASE(SYS_init_buffers);
    CASE(SYS_flush_buffers);
    CASE(SYS_share_name);
    CASE(SYS_enable_dump);
    CASE(SYS_update_maps);
    CASE(SYS_dump_x11);
    CASE(SYS_dump_dbus);

#if !defined(__aarch64__)
    CASE(SYS_sysfs);
    CASE(SYS_get_kernel_syms);
    CASE(SYS_query_module);
    CASE(SYS_afs_syscall);
    CASE(SYS_vserver);
#endif

#if defined(__aarch64__)
    CASE(SYS_fstatat);
#elif defined(__x86_64__) || defined(__mips64)
    CASE(SYS_shmat);
    CASE(SYS_pause);
    CASE(SYS_alarm);
    CASE(SYS_getpid);
    CASE(SYS_creat);
    CASE(SYS_getuid);
    CASE(SYS_getgid);
    CASE(SYS_geteuid);
    CASE(SYS_getegid);
    CASE(SYS_getppid);
    CASE(SYS_utime);
    CASE(SYS_umount2);
    CASE(SYS_create_module);
    CASE(SYS_getpmsg);
    CASE(SYS_putpmsg);
    CASE(SYS_set_thread_area);
    CASE(SYS_utimes);
    CASE(SYS_newfstatat);
    CASE(SYS_utimensat);

#ifndef KYLIN_MIPS64
    CASE(SYS_seccomp);
    CASE(SYS_bpf);
    CASE(SYS_userfaultfd);
    CASE(SYS_membarrier);
    CASE(SYS_mlock2);
#endif

#elif defined(__sw_64)
    CASE(SYS_bdflush);
    CASE(SYS_create_module);
    CASE(SYS_dipc);
    CASE(SYS_exec_with_loader);
    CASE(SYS_fstat64);
    CASE(SYS_fstatat64);
    CASE(SYS_getdtablesize);
    CASE(SYS_gethostname);
    CASE(SYS_getpagesize);
    CASE(SYS_getxgid);
    CASE(SYS_getxpid);
    CASE(SYS_getxuid);
    CASE(SYS_lstat64);
    CASE(SYS_old_adjtimex);
    CASE(SYS_oldumount);
    CASE(SYS_osf_adjtime);
    CASE(SYS_osf_afs_syscall);
    CASE(SYS_osf_alt_plock);
    CASE(SYS_osf_alt_setsid);
    CASE(SYS_osf_alt_sigpending);
    CASE(SYS_osf_asynch_daemon);
    CASE(SYS_osf_audcntl);
    CASE(SYS_osf_audgen);
    CASE(SYS_osf_chflags);
    CASE(SYS_osf_execve);
    CASE(SYS_osf_exportfs);
    CASE(SYS_osf_fchflags);
    CASE(SYS_osf_fdatasync);
    CASE(SYS_osf_fpathconf);
    CASE(SYS_osf_fstat);
    CASE(SYS_osf_fstatfs);
    CASE(SYS_osf_fstatfs64);
    CASE(SYS_osf_fuser);
    CASE(SYS_osf_getaddressconf);
    CASE(SYS_osf_getdirentries);
    CASE(SYS_osf_getdomainname);
    CASE(SYS_osf_getfh);
    CASE(SYS_osf_getfsstat);
    CASE(SYS_osf_gethostid);
    CASE(SYS_osf_getitimer);
    CASE(SYS_osf_getlogin);
    CASE(SYS_osf_getmnt);
    CASE(SYS_osf_getrusage);
    CASE(SYS_osf_getsysinfo);
    CASE(SYS_osf_gettimeofday);
    CASE(SYS_osf_kloadcall);
    CASE(SYS_osf_kmodcall);
    CASE(SYS_osf_lstat);
    CASE(SYS_osf_memcntl);
    CASE(SYS_osf_mincore);
    CASE(SYS_osf_mount);
    CASE(SYS_osf_mremap);
    CASE(SYS_osf_msfs_syscall);
    CASE(SYS_osf_msleep);
    CASE(SYS_osf_mvalid);
    CASE(SYS_osf_mwakeup);
    CASE(SYS_osf_naccept);
    CASE(SYS_osf_nfssvc);
    CASE(SYS_osf_ngetpeername);
    CASE(SYS_osf_ngetsockname);
    CASE(SYS_osf_nrecvfrom);
    CASE(SYS_osf_nrecvmsg);
    CASE(SYS_osf_nsendmsg);
    CASE(SYS_osf_ntp_adjtime);
    CASE(SYS_osf_ntp_gettime);
    CASE(SYS_osf_old_creat);
    CASE(SYS_osf_old_fstat);
    CASE(SYS_osf_old_getpgrp);
    CASE(SYS_osf_old_killpg);
    CASE(SYS_osf_old_lstat);
    CASE(SYS_osf_old_open);
    CASE(SYS_osf_old_sigaction);
    CASE(SYS_osf_old_sigblock);
    CASE(SYS_osf_old_sigreturn);
    CASE(SYS_osf_old_sigsetmask);
    CASE(SYS_osf_old_sigvec);
    CASE(SYS_osf_old_stat);
    CASE(SYS_osf_old_vadvise);
    CASE(SYS_osf_old_vtrace);
    CASE(SYS_osf_old_wait);
    CASE(SYS_osf_oldquota);
    CASE(SYS_osf_pathconf);
    CASE(SYS_osf_pid_block);
    CASE(SYS_osf_pid_unblock);
    CASE(SYS_osf_plock);
    CASE(SYS_osf_priocntlset);
    CASE(SYS_osf_profil);
    CASE(SYS_osf_proplist_syscall);
    CASE(SYS_osf_reboot);
    CASE(SYS_osf_revoke);
    CASE(SYS_osf_sbrk);
    CASE(SYS_osf_security);
    CASE(SYS_osf_select);
    CASE(SYS_osf_set_program_attributes);
    CASE(SYS_osf_set_speculative);
    CASE(SYS_osf_sethostid);
    CASE(SYS_osf_setitimer);
    CASE(SYS_osf_setlogin);
    CASE(SYS_osf_setsysinfo);
    CASE(SYS_osf_settimeofday);
    CASE(SYS_osf_shmat);
    CASE(SYS_osf_signal);
    CASE(SYS_osf_sigprocmask);
    CASE(SYS_osf_sigsendset);
    CASE(SYS_osf_sigstack);
    CASE(SYS_osf_sigwaitprim);
    CASE(SYS_osf_sstk);
    CASE(SYS_osf_stat);
    CASE(SYS_osf_statfs);
    CASE(SYS_osf_statfs64);
    CASE(SYS_osf_subsys_info);
    CASE(SYS_osf_swapctl);
    CASE(SYS_osf_swapon);
    CASE(SYS_osf_syscall);
    CASE(SYS_osf_sysinfo);
    CASE(SYS_osf_table);
    CASE(SYS_osf_uadmin);
    CASE(SYS_osf_usleep_thread);
    CASE(SYS_osf_uswitch);
    CASE(SYS_osf_utc_adjtime);
    CASE(SYS_osf_utc_gettime);
    CASE(SYS_osf_utimes);
    CASE(SYS_osf_utsname);
    CASE(SYS_osf_wait4);
    CASE(SYS_osf_waitid);
    CASE(SYS_pciconfig_iobase);
    CASE(SYS_pciconfig_read);
    CASE(SYS_pciconfig_write);
    CASE(SYS_recv);
    CASE(SYS_select);
    CASE(SYS_send);
    CASE(SYS_sethae);
    CASE(SYS_setpgrp);
    CASE(SYS_sigaction);
    CASE(SYS_sigpending);
    CASE(SYS_sigreturn);
    CASE(SYS_sigsuspend);
    CASE(SYS_stat64);
    CASE(SYS_timerfd);
    CASE(SYS_tuxcall);
    CASE(SYS_umount);
    CASE(SYS_uselib);
    CASE(SYS_utimensat);
    CASE(SYS_utimes);
    CASE(SYS_vfork);
#endif

  default:
    {
        static char buf[32];
        sprintf(buf, "UNKN:%ld", call);
        return buf;
    }
  }
}

static const char* x11_event_name[] = {
#if 0 // Must implement
    "KeyPress",
    "KeyRelease",
    "ButtonPress",
    "ButtonRelease",
    "MotionNotify",
    "FocusIn",
    "FocusOut",
    "CreateNotify",
    "DestroyNotify",
    "MapNotify",
    "UnmapNotify",
#else // see /usr/include/X11/X.h
    "Error",
    "Reply",
    "KeyPress",           //2
    "KeyRelease",         //3
    "ButtonPress",        //4
    "ButtonRelease",      //5
    "MotionNotify",       //6
    "EnterNotify",        //7
    "LeaveNotify",        //8
    "FocusIn",            //9
    "FocusOut",           //10
    "KeymapNotify",       //11
    "Expose",             //12
    "GraphicsExpose",     //13
    "NoExpose",           //14
    "VisibilityNotify",   //15
    "CreateNotify",       //16
    "DestroyNotify",      //17
    "UnmapNotify",        //18
    "MapNotify",          //19
#endif
};

static const char* dbus_msg_name[] = {
    "invalid-message",
    "method-call",
    "method-return",
    "method-error",
    "send-signal",
    "receive-signal",
};

const char* get_event_name(int type)
{
    if (type < __NR_Linux) {
        return syscall_name(type - DUMP_REASON_syscall_exit);
    }
    else if ( type < DUMP_REASON_signal) {
        return syscall_name(type);
    }
    else if ( type < DUMP_REASON_dbus) {
        int sig = type - DUMP_REASON_signal;
        return signal_name(sig);
    }
    else if ( type < DUMP_REASON_x11) {
        unsigned int msg = type - DUMP_REASON_dbus;
        if (msg < sizeof(dbus_msg_name)/sizeof(dbus_msg_name[0]))
            return dbus_msg_name[msg];
        else
            return dbus_msg_name[0];
    }
    else if ( type < DUMP_REASON_ptrace) {
        unsigned int x11 = type - DUMP_REASON_x11;
        if (x11 < sizeof(x11_event_name)/sizeof(x11_event_name[0]))
            return x11_event_name[x11];
        else
            return x11_event_name[0];
    }
    else {
        return "Unknown";
    }

    return nullptr;
}
