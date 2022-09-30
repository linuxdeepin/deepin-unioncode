## 依赖的第三方库
* [libunwind](https://github.com/libunwind/libunwind)
* [facebook zstd](https://github.com/facebook/zstd/tree/dev/examples)
* libelfin,  https://github.com/TartanLlama/libelfin.git
* json-c, apt-get install libjson-c-dev
* libelf, apt-get install libelf-dev
* capstone, apt-get install libcapstone libcapstone-dev
* libdbus, apt-get install libdbus-1-dev 
* libx11, apt-get install libx11-dev
* libxcb, apt-get install libx11-xcb-dev
* x11 extension, apt-get install libxi-dev

##  配置文件
* 配置文件保存在 ~/.config/emd.json

* json格式,基本的内容如下:

```
{
    "shared_buffer_size": 4000,
    "max_dump_bytes": 4000000,
    "dump_dir": "/tmp/",
    "modules": [
            "/usr/lib/x86_64-linux-gnu/libqt*so*",
            "/usr/lib/x86_64-linux-gnu/libc-2.24.so"
    ],
    "variables": [
        {
            "name": "g_buf",
            "is_pointer: "true",
            "max_size": 256
        },
        {
            "name": "g_short,
            "is_pointer": "false",
            "max_size": 2
        }
    ],
    "compress_level":3,
    "log_debug": false,
    "log_to_stdout": false,
    "log_to_file": true,
    "log_file_max_size": 20000,
    "log_flush_threshold": 4
}
```
* shared_buffer_size, 指定进程内拦截syscall的共享数据区大小，单位为KB，默认是4MB

* max_dump_bytes, 指定最多记录的总字节数，单位为KB，默认是4GB

* dump_dir, 指定要转储的文件目录路径, 默认是 /home/your-name/.local/share/emd/

* heap, 指定是否转储 [heap] 内存段的数据,仅仅 hard 模式下有效果.

* modules 数组, 指定可写内存段要被转储的模块名:
    - 模块名称可以写完整的文件路径，也可以只写文件名，区分大小写, 支持通配符*和?
    - * 匹配0到N个任意字符(/除外,并且/之后的第一个字符不能是*)
    - ? 匹配1个任意字符(/除外,并且/之后的第一个字符不能是?)

* variables 数组,指定要转储的全局变量
    - name 指定全局变量名称,必须有调试符号.
    - max_size 指定全局变量转储的最大内存值,单位是字节.
    - is_pointer 是否是一个指针,如果是一个指针,max_size就是这个指针指向的内存大小.
      根据局部相关性,一个对象的指针成员分配的内存数据应该在这个指针的附近.

* compress_level, 指定转储的文件的压缩级别,级别从 1到19, 级别越大压缩率越高.

* log_debug, 指定是否记录debug日志，默认关闭.
* log_to_stdout,指定是否写到stdout，默认关闭.
* log_to_file, 指定是否写日志文件,默认开启
* log_file_max_size,指定每个日志文件的最大字节数，单位是KB，默认是20MB
* log_flush_threshold,指定每个日志文件缓存了多少字节后写到磁盘，单位是KB，
  默认是0，即立即写到磁盘.

这样熟悉源码的开发人员可以事先编写好dump配置文件，现场人员用这个配置文件进行事件记录,
有效减少记录文件的大小.

## 转储的数据文件格式
* 文件以每个线程一个记录块单位，线程记录块格式如下,总是4字节对齐:


```
 time(sizeof(struct timespec)), event_type(2byte), thread_num(2byte),
 current_thread_id(2byte), event_extra_size(2byte)[,event_extra_data]
 {
    tid(2byte), cpu_context_size(2byte)[, user_regs_struct+user_fpregs_struct],
    tls_size(4byte)[, tls_data]
    stack_addr(uintptr_t), stack_length(4byte)[,stack_data]
 }

 ...

 {
    tid(2byte), cpu_context_size(2byte)[, user_regs_struct+user_fpregs_struct],
    tls_size(4byte)[, tls_data]
    stack_addr(uintptr_t), stack_length(4byte)[,stack_data]
 }
 heap_count(4byte)[, [addr(uintptr_t)+size(4byte)+data], ...]
```


* reason, 指定转储的原因(系统调用，信号，事件等). 与事件无关的其它线程的reason值为0xffff,
  并且 time字段跟发生事件的线程的time字段一样. 如果是系统调用退出事件只记录系统调用返回值
  和errno.

* event_extra_data, 对x11事件来说,主要是 XEvent 结构体,包含事件相关的Window的句柄,
  窗口标题,窗口坐标;对dbus事件来说, 主要是 DBusMessage 结构体,包含消息的头和主体.

* tls_data,　记录了线程的局部存储数据.

* heap_count, 记录分配在堆内存的虚拟内存段个数和每个内存段的数据.

* 修改记录的数据格式后需要同时修改 record_block.cc, trace_reader.cc, session.cc,
  replay.cc, lib/event_dump.cc 文件
  里面的相关读写函数,否则会引起数据不一致.

## /proc/#pid/maps 文件
考虑到这个文件的内容只在这5个系统调用的时候发生改变:
`SYS_execve SYS_mmap SYS_munmap SYS_mprotect SYS_remap`


因此性能上考虑，就只在这4个情况下转储一次/proc/#pid/maps文件，并压缩到一个独立的文件里面.
每次转储的/proc/#pid/maps文件间用 "0\ntime_spec\n" 分隔.
详细处理方法见代码的 `TraceSession::dump_maps` 函数.

## 假定的规则
* 为了简单处理, 系统调用发生前记录(线程栈, cpu上下文, 线程局部存储), 系统调用结束后只记录
  系统调用结果.
* fast 模式仅仅记录当前线程的上下文信息.
* hard 模式可以记录当前线程的上下文信息或其它处于 trace-stop 状态的线程的上下文信息,
  以及堆内存段，全局变量; 不处于 trace-stop 的线程还在用户态运行,无需用 PTRACE-INTERRUPT 中断,
  有bug 肯定会崩溃或死锁, 在下一次事件就会记录到, 这样处理节约数据, 提高记录的性能.
* fast模式下, x11/dbus 看线程调用栈没有意义(总是x11/dbus 后台监视线程), 所有 stack-size 总是为０;
  但是可以查看 x11/dbus 事件参数和结果信息.
* fast 模式和 hard 模式是根据输入的参数自动选择的.

## 不同平台差别
* 检查cpu架构: `uname -m`
* 检查cpu字节顺序: `lscpu | grep "Byte Order"`
* 检查gcc预定义的架构宏:  `gcc -dM -E - < /dev/null | grep 64`
* cpu寄存器结构体在内核源码:
     arch/x86/include/uapi/asm/ptrace.h
     arch/mips/include/uapi/asm/ptrace.h
     arch/sw_64/include/uapi/asm/ptrace.h

## 龙芯平台 fast 模式下拦截syscall指令额外创建２个文件:
* /home/xxx/.config/libc.so.6.table 
* /home/xxx/.config/libpthread.so.0.table
分别用这个命令生成的:
`objdump -t debug-file | grep -- "\.text" | awk '{ print $1}' | sort | uniq`
其中的 debug-file 分别是 libc.so.6 和 libpthread.so.0 对应的调试文件保存在这个目录 /usr/lib/debug/.build-id/xx/yy

## 堆栈回溯只能看到几个函数帧
需要被调试的程序使用的第三方库有调试符号，特别是 qt 开发的界面程序，需要所有qt库的调试符号.
