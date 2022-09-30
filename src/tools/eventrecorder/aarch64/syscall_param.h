/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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
// flag bit 7 is set 0, as const char*;
// flag bit 7 is set 1, as void*, size indicate in next-arg;
unsigned char syscall_param_flags[] = {
0,2, //io_setup=0
0,1, //io_destroy=1
0,3, //io_submit=2
0,3, //io_cancel=3
0,5, //io_getevents=4
3, 5, //setxattr=5
3, 5, //lsetxattr=6
2, 5, //fsetxattr=7
3, 4, //getxattr=8
3, 4, //lgetxattr=9
2, 4, //fgetxattr=10
1, 3, //listxattr=11
1, 3, //llistxattr=12
0,3, //flistxattr=13
3, 2, //removexattr=14
3, 2, //lremovexattr=15
2, 2, //fremovexattr=16
0,2, //getcwd=17
0,3, //lookup_dcookie=18
0,2, //eventfd2=19
0,1, //epoll_create1=20
0,4, //epoll_ctl=21
0,6, //epoll_pwait=22
0,1, //dup=23
0,3, //dup3=24
0,3, //fcntl=25
0,1, //inotify_init1=26
2, 3, //inotify_add_watch=27
0,2, //inotify_rm_watch=28
0,3, //ioctl=29
0,3, //ioprio_set=30
0,2, //ioprio_get=31
0,2, //flock=32
2, 4, //mknodat=33
2, 3, //mkdirat=34
2, 3, //unlinkat=35
5, 3, //symlinkat=36
10, 5, //linkat=37
10, 4, //renameat=38
1, 2, //umount2=39
7, 5, //mount=40
3, 2, //pivot_root=41
0,3, //nfsservctl=42
1, 2, //statfs=43
0,2, //fstatfs=44
1, 2, //truncate=45
0,2, //ftruncate=46
0,4, //fallocate=47
0,3, //faccessat=48
1, 1, //chdir=49
0,1, //fchdir=50
1, 1, //chroot=51
0,2, //fchmod=52
0,3, //fchmodat=53
2, 5, //fchownat=54
0, 5, //fchown=55
2, 4, //openat=56
0,1, //close=57
0,0, //vhangup=58
0,2, //pipe2=59
2, 4, //quotactl=60
0,3, //getdents64=61
0,3, //lseek=62
0x82, 3, //read=63
0x82, 3, //write=64
0,3, //readv=65
0,3, //writev=66
0x82, 4, //pread64=67
0x82, 4, //pwrite64=68
0,4, //preadv=69
0,4, //pwritev=70
0,4, //sendfile=71
0,6, //pselect6=72
0,5, //ppoll=73
0,4, //signalfd4=74
0,4, //vmsplice=75
0,6, //splice=76
0,4, //tee=77
2, 4, //readlinkat=78
2, 4, //fstatat=79
0, 4, //fstat=80
0,0, //sync=81
0,1, //fsync=82
0,1, //fdatasync=83
0,4, //sync_file_range=84
0,2, //timerfd_create=85
0,4, //timerfd_settime=86
0,2, //timerfd_gettime=87
0,4, //utimensat=88
1, 1, //acct=89
0,2, //capget=90
0,2, //capset=91
0,1, //personality=92
0,1, //exit=93
0,1, //exit_group=94
0,5, //waitid=95
0,1, //set_tid_address=96
0,1, //unshare=97
0,6, //futex=98
0,2, //set_robust_list=99
0,3, //get_robust_list=100
0,2, //nanosleep=101
0,2, //getitimer=102
0,3, //setitimer=103
0,4, //kexec_load=104
4, 3, //init_module=105
1, 2, //delete_module=106
0,3, //timer_create=107
0,2, //timer_gettime=108
0,1, //timer_getoverrun=109
0,4, //timer_settime=110
0,1, //timer_delete=111
0,2, //clock_settime=112
0,2, //clock_gettime=113
0,2, //clock_getres=114
0,4, //clock_nanosleep=115
0,3, //syslog=116
0,4, //ptrace=117
0,2, //sched_setparam=118
0,3, //sched_setscheduler=119
0,1, //sched_getscheduler=120
0,2, //sched_getparam=121
0,3, //sched_setaffinity=122
0,3, //sched_getaffinity=123
0,0, //sched_yield=124
0,1, //sched_get_priority_max=125
0,1, //sched_get_priority_min=126
0,2, //sched_rr_get_interval=127
0,0, //restart_syscall=128
0,2, //kill=129
0,2, //tkill=130
0,3, //tgkill=131
0,2, //sigaltstack=132
0,2, //rt_sigsuspend=133
0,4, //rt_sigaction=134
0,4, //rt_sigprocmask=135
0,2, //rt_sigpending=136
0,4, //rt_sigtimedwait=137
0,3, //rt_sigqueueinfo=138
0,0, //rt_sigreturn=139
0,3, //setpriority=140
0,2, //getpriority=141
0,4, //reboot=142
0,2, //setregid=143
0,1, //setgid=144
0,2, //setreuid=145
0,1, //setuid=146
0,3, //setresuid=147
0,3, //getresuid=148
0,3, //setresgid=149
0,3, //getresgid=150
0,1, //setfsuid=151
0,1, //setfsgid=152
0, 2, //times=153
0,2, //setpgid=154
0,1, //getpgid=155
0,1, //getsid=156
0,0, //setsid=157
0,2, //getgroups=158
0,2, //setgroups=159
0,1, //uname=160
1, 2, //sethostname=161
1, 2, //setdomainname=162
0,2, //getrlimit=163
0,2, //setrlimit=164
0,2, //getrusage=165
0,1, //umask=166
0,5, //prctl=167
0,3, //getcpu=168
0,2, //gettimeofday=169
0,2, //settimeofday=170
0,1, //adjtimex=171
0,0, //getpid=172
0,0, //getppid=173
0,0, //getuid=174
0,0, //geteuid=175
0,0, //getgid=176
0,0, //getegid=177
0,0, //gettid=178
0,1, //sysinfo=179
1, 4, //mq_open=180
1, 1, //mq_unlink=181
2, 5, //mq_timedsend=182
0,5, //mq_timedreceive=183
0,2, //mq_notify=184
0,3, //mq_getsetattr=185
0,2, //msgget=186
0,3, //msgctl=187
0,5, //msgrcv=188
0,4, //msgsnd=189
0,3, //semget=190
0,4, //semctl=191
0,4, //semtimedop=192
0,3, //semop=193
0,3, //shmget=194
0,3, //shmctl=195
0,3, //shmat=196
0,1, //shmdt=197
0,3, //socket=198
0,4, //socketpair=199
0,3, //bind=200
0,2, //listen=201
0,3, //accept=202
0,3, //connect=203
0,3, //getsockname=204
0,3, //getpeername=205
0x82, 6, //sendto=206
0x82, 6, //recvfrom=207
0,5, //setsockopt=208
0,5, //getsockopt=209
0,2, //shutdown=210
0, 3, //sendmsg=211
0, 3, //recvmsg=212
0,3, //readahead=213
0,1, //brk=214
0,2, //munmap=215
0,5, //mremap=216
3, 5, //add_key=217
7, 4, //request_key=218
0,5, //keyctl=219
0,5, //clone=220
1, 3, //execve=221
0,6, //mmap=222
0,4, //fadvise64=223
1, 2, //swapon=224
1, 1, //swapoff=225
0,3, //mprotect=226
0,3, //msync=227
0,2, //mlock=228
0,2, //munlock=229
0,1, //mlockall=230
0,0, //munlockall=231
0,3, //mincore=232
0,3, //madvise=233
0,5, //remap_file_pages=234
0,6, //mbind=235
0,5, //get_mempolicy=236
0,3, //set_mempolicy=237
0,4, //migrate_pages=238
0,6, //move_pages=239
0,4, //rt_tgsigqueueinfo=240
0,5, //perf_event_open=241
0,4, //accept4=242
0,5, //recvmmsg=243
0,0, //not_used_244
0,0, //not_used_245
0,0, //not_used_246
0,0, //not_used_247
0,0, //not_used_248
0,0, //not_used_249
0,0, //not_used_250
0,0, //not_used_251
0,0, //not_used_252
0,0, //not_used_253
0,0, //not_used_254
0,0, //not_used_255
0,0, //not_used_256
0,0, //not_used_257
0,0, //not_used_258
0,0, //not_used_259
0,4, //wait4=260
0,4, //prlimit64=261
0,2, //fanotify_init=262
0,5, //fanotify_mark=263
0,5, //name_to_handle_at=264
0,3, //open_by_handle_at=265
0,2, //clock_adjtime=266
0,1, //syncfs=267
0,2, //setns=268
0,4, //sendmmsg=269
0,6, //process_vm_readv=270
0,6, //process_vm_writev=271
0,5, //kcmp=272
0,3, //finit_module=273
0,3, //sched_setattr=274
0,4, //sched_getattr=275
0,5, //renameat2=276
0,3, //seccomp=277
0,3, //getrandom=278
0,2, //memfd_create=279
0,3, //bpf=280
0,5, //execveat=281
0,1, //userfaultfd=282
0,2, //membarrier=283
0,3, //mlock2=284
0,6, //copy_file_range=285
0,6, //preadv2=286
0,6, //pwritev2=287
0,4, //pkey_mprotect=288
0,2, //pkey_alloc=289
0,1, //pkey_free=290
0,0, //not_used_291
0,0, //not_used_292
0,0, //not_used_293
0,0, //not_used_294
0,0, //not_used_295
0,0, //not_used_296
0,0, //not_used_297
0,0, //not_used_298
0,0, //not_used_299
0,0, //not_used_300
0,0, //not_used_301
0,0, //not_used_302
0,0, //not_used_303
0,0, //not_used_304
0,0, //not_used_305
0,0, //not_used_306
0,0, //not_used_307
0,0, //not_used_308
0,0, //not_used_309
0,0, //not_used_310
0,0, //not_used_311
0,0, //not_used_312
0,0, //not_used_313
0,0, //not_used_314
0,0, //not_used_315
0,0, //not_used_316
0,0, //not_used_317
0,0, //not_used_318
0,0, //not_used_319
0,0, //not_used_320
0,0, //not_used_321
0,0, //not_used_322
0,0, //not_used_323
0,0, //not_used_324
0,0, //not_used_325
0,0, //not_used_326
0,0, //not_used_327
0,0, //not_used_328
0,0, //not_used_329
0,0, //not_used_330
0,0, //not_used_331
0,0, //not_used_332
0,0, //not_used_333
0,0, //not_used_334
0,0, //not_used_335
0,0, //not_used_336
0,0, //not_used_337
0,0, //not_used_338
0,0, //not_used_339
0,0, //not_used_340
0,0, //not_used_341
0,0, //not_used_342
0,0, //not_used_343
0,0, //not_used_344
0,0, //not_used_345
0,0, //not_used_346
0,0, //not_used_347
0,0, //not_used_348
0,0, //not_used_349
0,0, //not_used_350
0,0, //not_used_351
0,0, //not_used_352
0,0, //not_used_353
0,0, //not_used_354
0,0, //not_used_355
0,0, //not_used_356
0,0, //not_used_357
0,0, //not_used_358
0,0, //not_used_359
0,0, //not_used_360
0,0, //not_used_361
0,0, //not_used_362
0,0, //not_used_363
0,0, //not_used_364
0,0, //not_used_365
0,0, //not_used_366
0,0, //not_used_367
0,0, //not_used_368
0,0, //not_used_369
0,0, //not_used_370
0,0, //not_used_371
0,0, //not_used_372
0,0, //not_used_373
0,0, //not_used_374
0,0, //not_used_375
0,0, //not_used_376
0,0, //not_used_377
0,0, //not_used_378
0,0, //not_used_379
0,0, //not_used_380
0,0, //not_used_381
0,0, //not_used_382
0,0, //not_used_383
0,0, //not_used_384
0,0, //not_used_385
0,0, //not_used_386
0,0, //not_used_387
0,0, //not_used_388
0,0, //not_used_389
0,0, //not_used_390
0,0, //not_used_391
0,0, //not_used_392
0,0, //not_used_393
0,0, //not_used_394
0,0, //not_used_395
0,0, //not_used_396
0,0, //not_used_397
0,0, //not_used_398
0,0, //not_used_399
0,0, //not_used_400
0,0, //not_used_401
0,0, //not_used_402
0,0, //not_used_403
0,0, //not_used_404
0,0, //not_used_405
0,0, //not_used_406
0,0, //not_used_407
0,0, //not_used_408
0,0, //not_used_409
0,0, //not_used_410
0,0, //not_used_411
0,0, //not_used_412
0,0, //not_used_413
0,0, //not_used_414
0,0, //not_used_415
0,0, //not_used_416
0,0, //not_used_417
0,0, //not_used_418
0,0, //not_used_419
0,0, //not_used_420
0,0, //not_used_421
0,0, //not_used_422
0,0, //not_used_423
0,0, //not_used_424
0,0, //not_used_425
0,0, //not_used_426
0,0, //not_used_427
0,0, //not_used_428
0,0, //not_used_429
0,0, //not_used_430
0,0, //not_used_431
0,0, //not_used_432
0,0, //not_used_433
0,0, //not_used_434
0,0, //not_used_435
0,0, //not_used_436
0,0, //not_used_437
0,0, //not_used_438
0,0, //not_used_439
0,0, //not_used_440
0,0, //not_used_441
0,0, //not_used_442
0,0, //not_used_443
0,0, //not_used_444
0,0, //not_used_445
0,0, //not_used_446
0,0, //not_used_447
0,0, //not_used_448
0,0, //not_used_449
0,0, //not_used_450
0,0, //not_used_451
0,0, //not_used_452
0,0, //not_used_453
0,0, //not_used_454
0,0, //not_used_455
0,0, //not_used_456
0,0, //not_used_457
0,0, //not_used_458
0,0, //not_used_459
0,0, //not_used_460
0,0, //not_used_461
0,0, //not_used_462
0,0, //not_used_463
0,0, //not_used_464
0,0, //not_used_465
0,0, //not_used_466
0,0, //not_used_467
0,0, //not_used_468
0,0, //not_used_469
0,0, //not_used_470
0,0, //not_used_471
0,0, //not_used_472
0,0, //not_used_473
0,0, //not_used_474
0,0, //not_used_475
0,0, //not_used_476
0,0, //not_used_477
0,0, //not_used_478
0,0, //not_used_479
0,0, //not_used_480
0,0, //not_used_481
0,0, //not_used_482
0,0, //not_used_483
0,0, //not_used_484
0,0, //not_used_485
0,0, //not_used_486
0,0, //not_used_487
0,0, //not_used_488
0,0, //not_used_489
0,0, //not_used_490
0,0, //not_used_491
0,0, //not_used_492
0,0, //not_used_493
0,0, //not_used_494
0,0, //not_used_495
0,0, //not_used_496
0,0, //not_used_497
0,0, //not_used_498
0,0, //not_used_499
0,0, //not_used_500
0,0, //not_used_501
0,0, //not_used_502
0,0, //not_used_503
0,0, //not_used_504
0,0, //not_used_505
0,0, //not_used_506
0,0, //not_used_507
0,0, //not_used_508
0,0, //not_used_509
0,0, //not_used_510
0,0, //not_used_511
0,0, //not_used_512
0,0, //not_used_513
0,0, //not_used_514
0,0, //not_used_515
0,0, //not_used_516
0,0, //not_used_517
0,0, //not_used_518
0,0, //not_used_519
0,0, //not_used_520
0,0, //not_used_521
0,0, //not_used_522
0,0, //not_used_523
0,0, //not_used_524
0,0, //not_used_525
0,0, //not_used_526
0,0, //not_used_527
0,0, //not_used_528
0,0, //not_used_529
0,0, //not_used_530
0,0, //not_used_531
0,0, //not_used_532
0,0, //not_used_533
0,0, //not_used_534
0,0, //not_used_535
0,0, //not_used_536
0,0, //not_used_537
0,0, //not_used_538
0,0, //not_used_539
0,0, //not_used_540
0,0, //not_used_541
0,0, //not_used_542
0,0, //not_used_543
0,0, //not_used_544
0,0, //not_used_545
0,0, //not_used_546
0,0, //not_used_547
0,0, //not_used_548
0,0, //not_used_549
0,0, //not_used_550
0,0, //not_used_551
0,0, //not_used_552
0,0, //not_used_553
0,0, //not_used_554
0,0, //not_used_555
0,0, //not_used_556
0,0, //not_used_557
0,0, //not_used_558
0,0, //not_used_559
0,0, //not_used_560
0,0, //not_used_561
0,0, //not_used_562
0,0, //not_used_563
0,0, //not_used_564
0,0, //not_used_565
0,0, //not_used_566
0,0, //not_used_567
0,0, //not_used_568
0,0, //not_used_569
0,0, //not_used_570
0,0, //not_used_571
0,0, //not_used_572
0,0, //not_used_573
0,0, //not_used_574
0,0, //not_used_575
0,0, //not_used_576
0,0, //not_used_577
0,0, //not_used_578
0,0, //not_used_579
0,0, //not_used_580
0,0, //not_used_581
0,0, //not_used_582
0,0, //not_used_583
0,0, //not_used_584
0,0, //not_used_585
0,0, //not_used_586
0,0, //not_used_587
0,0, //not_used_588
0,0, //not_used_589
0,0, //not_used_590
0,0, //not_used_591
0,0, //not_used_592
0,0, //not_used_593
0,0, //not_used_594
0,0, //not_used_595
0,0, //not_used_596
0,0, //not_used_597
0,0, //not_used_598
0,0, //not_used_599
0,0, //not_used_600
0,0, //not_used_601
0,0, //not_used_602
0,0, //not_used_603
0,0, //not_used_604
0,0, //not_used_605
0,0, //not_used_606
0,0, //not_used_607
0,0, //not_used_608
0,0, //not_used_609
0,0, //not_used_610
0,0, //not_used_611
0,0, //not_used_612
0,0, //not_used_613
0,0, //not_used_614
0,0, //not_used_615
0,0, //not_used_616
0,0, //not_used_617
0,0, //not_used_618
0,0, //not_used_619
0,0, //not_used_620
0,0, //not_used_621
0,0, //not_used_622
0,0, //not_used_623
0,0, //not_used_624
0,0, //not_used_625
0,0, //not_used_626
0,0, //not_used_627
0,0, //not_used_628
0,0, //not_used_629
0,0, //not_used_630
0,0, //not_used_631
0,0, //not_used_632
0,0, //not_used_633
0,0, //not_used_634
0,0, //not_used_635
0,0, //not_used_636
0,0, //not_used_637
0,0, //not_used_638
0,0, //not_used_639
0,0, //not_used_640
0,0, //not_used_641
0,0, //not_used_642
0,0, //not_used_643
0,0, //not_used_644
0,0, //not_used_645
0,0, //not_used_646
0,0, //not_used_647
0,0, //not_used_648
0,0, //not_used_649
0,0, //not_used_650
0,0, //not_used_651
0,0, //not_used_652
0,0, //not_used_653
0,0, //not_used_654
0,0, //not_used_655
0,0, //not_used_656
0,0, //not_used_657
0,0, //not_used_658
0,0, //not_used_659
0,0, //not_used_660
0,0, //not_used_661
0,0, //not_used_662
0,0, //not_used_663
0,0, //not_used_664
0,0, //not_used_665
0,0, //not_used_666
0,0, //not_used_667
0,0, //not_used_668
0,0, //not_used_669
0,0, //not_used_670
0,0, //not_used_671
0,0, //not_used_672
0,0, //not_used_673
0,0, //not_used_674
0,0, //not_used_675
0,0, //not_used_676
0,0, //not_used_677
0,0, //not_used_678
0,0, //not_used_679
0,0, //not_used_680
0,0, //not_used_681
0,0, //not_used_682
0,0, //not_used_683
0,0, //not_used_684
0,0, //not_used_685
0,0, //not_used_686
0,0, //not_used_687
0,0, //not_used_688
0,0, //not_used_689
0,0, //not_used_690
0,0, //not_used_691
0,0, //not_used_692
0,0, //not_used_693
0,0, //not_used_694
0,0, //not_used_695
0,0, //not_used_696
0,0, //not_used_697
0,0, //not_used_698
0,0, //not_used_699
0,0, //not_used_700
0,0, //not_used_701
0,0, //not_used_702
0,0, //not_used_703
0,0, //not_used_704
0,0, //not_used_705
0,0, //not_used_706
0,0, //not_used_707
0,0, //not_used_708
0,0, //not_used_709
0,0, //not_used_710
0,0, //not_used_711
0,0, //not_used_712
0,0, //not_used_713
0,0, //not_used_714
0,0, //not_used_715
0,0, //not_used_716
0,0, //not_used_717
0,0, //not_used_718
0,0, //not_used_719
0,0, //not_used_720
0,0, //not_used_721
0,0, //not_used_722
0,0, //not_used_723
0,0, //not_used_724
0,0, //not_used_725
0,0, //not_used_726
0,0, //not_used_727
0,0, //not_used_728
0,0, //not_used_729
0,0, //not_used_730
0,0, //not_used_731
0,0, //not_used_732
0,0, //not_used_733
0,0, //not_used_734
0,0, //not_used_735
0,0, //not_used_736
0,0, //not_used_737
0,0, //not_used_738
0,0, //not_used_739
0,0, //not_used_740
0,0, //not_used_741
0,0, //not_used_742
0,0, //not_used_743
0,0, //not_used_744
0,0, //not_used_745
0,0, //not_used_746
0,0, //not_used_747
0,0, //not_used_748
0,0, //not_used_749
0,0, //not_used_750
0,0, //not_used_751
0,0, //not_used_752
0,0, //not_used_753
0,0, //not_used_754
0,0, //not_used_755
0,0, //not_used_756
0,0, //not_used_757
0,0, //not_used_758
0,0, //not_used_759
0,0, //not_used_760
0,0, //not_used_761
0,0, //not_used_762
0,0, //not_used_763
0,0, //not_used_764
0,0, //not_used_765
0,0, //not_used_766
0,0, //not_used_767
0,0, //not_used_768
0,0, //not_used_769
0,0, //not_used_770
0,0, //not_used_771
0,0, //not_used_772
0,0, //not_used_773
0,0, //not_used_774
0,0, //not_used_775
0,0, //not_used_776
0,0, //not_used_777
0,0, //not_used_778
0,0, //not_used_779
0,0, //not_used_780
0,0, //not_used_781
0,0, //not_used_782
0,0, //not_used_783
0,0, //not_used_784
0,0, //not_used_785
0,0, //not_used_786
0,0, //not_used_787
0,0, //not_used_788
0,0, //not_used_789
0,0, //not_used_790
0,0, //not_used_791
0,0, //not_used_792
0,0, //not_used_793
0,0, //not_used_794
0,0, //not_used_795
0,0, //not_used_796
0,0, //not_used_797
0,0, //not_used_798
0,0, //not_used_799
0,0, //not_used_800
0,0, //not_used_801
0,0, //not_used_802
0,0, //not_used_803
0,0, //not_used_804
0,0, //not_used_805
0,0, //not_used_806
0,0, //not_used_807
0,0, //not_used_808
0,0, //not_used_809
0,0, //not_used_810
0,0, //not_used_811
0,0, //not_used_812
0,0, //not_used_813
0,0, //not_used_814
0,0, //not_used_815
0,0, //not_used_816
0,0, //not_used_817
0,0, //not_used_818
0,0, //not_used_819
0,0, //not_used_820
0,0, //not_used_821
0,0, //not_used_822
0,0, //not_used_823
0,0, //not_used_824
0,0, //not_used_825
0,0, //not_used_826
0,0, //not_used_827
0,0, //not_used_828
0,0, //not_used_829
0,0, //not_used_830
0,0, //not_used_831
0,0, //not_used_832
0,0, //not_used_833
0,0, //not_used_834
0,0, //not_used_835
0,0, //not_used_836
0,0, //not_used_837
0,0, //not_used_838
0,0, //not_used_839
0,0, //not_used_840
0,0, //not_used_841
0,0, //not_used_842
0,0, //not_used_843
0,0, //not_used_844
0,0, //not_used_845
0,0, //not_used_846
0,0, //not_used_847
0,0, //not_used_848
0,0, //not_used_849
0,0, //not_used_850
0,0, //not_used_851
0,0, //not_used_852
0,0, //not_used_853
0,0, //not_used_854
0,0, //not_used_855
0,0, //not_used_856
0,0, //not_used_857
0,0, //not_used_858
0,0, //not_used_859
0,0, //not_used_860
0,0, //not_used_861
0,0, //not_used_862
0,0, //not_used_863
0,0, //not_used_864
0,0, //not_used_865
0,0, //not_used_866
0,0, //not_used_867
0,0, //not_used_868
0,0, //not_used_869
0,0, //not_used_870
0,0, //not_used_871
0,0, //not_used_872
0,0, //not_used_873
0,0, //not_used_874
0,0, //not_used_875
0,0, //not_used_876
0,0, //not_used_877
0,0, //not_used_878
0,0, //not_used_879
0,0, //not_used_880
0,0, //not_used_881
0,0, //not_used_882
0,0, //not_used_883
0,0, //not_used_884
0,0, //not_used_885
0,0, //not_used_886
0,0, //not_used_887
0,0, //not_used_888
0,0, //not_used_889
0,0, //not_used_890
0,0, //not_used_891
0,0, //not_used_892
0,0, //not_used_893
0,0, //not_used_894
0,0, //not_used_895
0,0, //not_used_896
0,0, //not_used_897
0,0, //not_used_898
0,0, //not_used_899
0,0, //not_used_900
0,0, //not_used_901
0,0, //not_used_902
0,0, //not_used_903
0,0, //not_used_904
0,0, //not_used_905
0,0, //not_used_906
0,0, //not_used_907
0,0, //not_used_908
0,0, //not_used_909
0,0, //not_used_910
0,0, //not_used_911
0,0, //not_used_912
0,0, //not_used_913
0,0, //not_used_914
0,0, //not_used_915
0,0, //not_used_916
0,0, //not_used_917
0,0, //not_used_918
0,0, //not_used_919
0,0, //not_used_920
0,0, //not_used_921
0,0, //not_used_922
0,0, //not_used_923
0,0, //not_used_924
0,0, //not_used_925
0,0, //not_used_926
0,0, //not_used_927
0,0, //not_used_928
0,0, //not_used_929
0,0, //not_used_930
0,0, //not_used_931
0,0, //not_used_932
0,0, //not_used_933
0,0, //not_used_934
0,0, //not_used_935
0,0, //not_used_936
0,0, //not_used_937
0,0, //not_used_938
0,0, //not_used_939
0,0, //not_used_940
0,0, //not_used_941
0,0, //not_used_942
0,0, //not_used_943
0,0, //not_used_944
0,0, //not_used_945
0,0, //not_used_946
0,0, //not_used_947
0,0, //not_used_948
0,0, //not_used_949
0,0, //not_used_950
0,0, //not_used_951
0,0, //not_used_952
0,0, //not_used_953
0,0, //not_used_954
0,0, //not_used_955
0,0, //not_used_956
0,0, //not_used_957
0,0, //not_used_958
0,0, //not_used_959
0,0, //not_used_960
0,0, //not_used_961
0,0, //not_used_962
0,0, //not_used_963
0,0, //not_used_964
0,0, //not_used_965
0,0, //not_used_966
0,0, //not_used_967
0,0, //not_used_968
0,0, //not_used_969
0,0, //not_used_970
0,0, //not_used_971
0,0, //not_used_972
0,0, //not_used_973
0,0, //not_used_974
0,0, //not_used_975
0,0, //not_used_976
0,0, //not_used_977
0,0, //not_used_978
0,0, //not_used_979
0,0, //not_used_980
0,0, //not_used_981
0,0, //not_used_982
0,0, //not_used_983
0,0, //not_used_984
0,0, //not_used_985
0,0, //not_used_986
0,0, //not_used_987
0,0, //not_used_988
0,0, //not_used_989
0,0, //not_used_990
0,0, //not_used_991
0,0, //not_used_992
0,0, //not_used_993
0,0, //not_used_994
0,0, //not_used_995
0,0, //not_used_996
0,0, //not_used_997
0,0, //not_used_998
0,0, //not_used_999
0,0, //not_used_1000
0,0, //not_used_1001
0,0, //not_used_1002
0,0, //not_used_1003
0,0, //not_used_1004
0,0, //not_used_1005
0,0, //not_used_1006
0,0, //not_used_1007
0,0, //not_used_1008
0,0, //not_used_1009
0,0, //not_used_1010
0,0, //not_used_1011
0,0, //not_used_1012
0,0, //not_used_1013
0,0, //not_used_1014
0,0, //not_used_1015
0,0, //not_used_1016
0,0, //not_used_1017
0,0, //not_used_1018
0,0, //not_used_1019
0,0, //not_used_1020
0,0, //not_used_1021
0,0, //not_used_1022
0,0, //not_used_1023
1, 3, //open=1024
3, 2, //link=1025
1, 1, //unlink=1026
1, 3, //mknod=1027
1, 2, //chmod=1028
1, 3, //chown=1029
1, 2, //mkdir=1030
1, 1, //rmdir=1031
1, 3, //lchown=1032
1, 2, //access=1033
3, 2, //rename=1034
1, 3, //readlink=1035
3, 2, //symlink=1036
1, 2, //utimes=1037
1, 2, //stat=1038
1, 2, //lstat=1039
0,1, //pipe=1040
0,2, //dup2=1041
0,1, //epoll_create=1042
0,0, //inotify_init=1043
0,1, //eventfd=1044
0,3, //signalfd=1045
0,4, //not_used=1046
0,2, //not_used=1047
1, 2, //not_used=1048
1, 2, //not_used=1049
1, 2, //not_used=1050
0, 0, //not_used=1051
0,3, //not_used=1052
0,4, //not_used=1053
2, 4, //not_used=1054
0,2, //not_used=1055
1, 2, //not_used=1056
0,3, //not_used=1057
0,6, //not_used=1058
0,1, //alarm=1059
0,0, //getpgrp=1060
0,0, //pause=1061
1, 2, //time=1062
1, 2, //utime=1063
1, 2, //creat=1064
0,3, //getdents=1065
2, 3, //futimesat=1066
0,5, //select=1067
0,3, //poll=1068
0,4, //epoll_wait=1069
0,2, //ustat=1070
0,0, //vfork=1071
0,4, //not_used=1072
0, 6, //not_used=1073
0, 6, //not_used=1074
0,2, //bdflush=1075
1, 2, //not_used=1076
1, 1, //uselib=1077
0,1, //sysctl=1078
0,0, //fork=1079
};
