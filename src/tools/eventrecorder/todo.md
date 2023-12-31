## TODO
* 一次线程切换也是一个事件,记录下来.

* replay 的 emd 控制台，加一个 count 命令，类似 list 命令，但是只列出指定事件总数.

* brk 系统调用会调用多次，如何同步更新 heap 区域

* fast 模式下搜索线程栈空间的时候是否要缓存起来? 搞一个map.
  https://github.com/rxi/map

* break-function 设置如果多个模块存在多个同名函数,怎么办?在名称前面加上模块名区别吧:  xxx.function 

* 配置文件可以设置只记录指定模块发起的系统调用，从而不记录libc等第三方库发起的系统调用

* 是否增加对系统调用的参数的多条件过滤:与或非逻辑.

* 申威的 getpid 是由 sys_call 0x9e 来实现的, 是否要拦截啊?

* 时间轴, 合并记录的数据时间显示,设置一个参数来控制重采样以提高绘图效率,比如:每个像素10s收集一次事件编号;
  时间轴如果显示绝对时间,跟事件列表的时间保持一致，时间刻度的数字就不是整数了.

* timeline 和 event list，联动要到什么程度呢? 或者参考 IDA吧:
    1. 事件窗口的工具栏按钮同时作用于时间轴和事件列表;
    2. 在事件列表或时间轴里面滚动,单击，双击不更新彼此的视图.

* 用 pref 评测fast,hard模式的性能差别,相差多少.

* fast模式: 如果子进程fork出来之后没有执行exec系统调用替换进程镜像文件，怎么办?
  通过检测pid可以知道是子进程还是父进程，但是如何知道子进程会不会执行exec呢?

* libunwind 看起来回溯不准啊,参考cfi信息看看:
  https://sourceware.org/binutils/docs/as/CFI-directives.html
  X86利用 .eh_frame 展开函数调用栈, ARM上利用 .ARM.exidx + .ARM.extab 展开调用栈.

* dbuspreload, x11preload里面使用的libc的某些函数会触发syscall吗?如何消除这种影响!
  统一调用 0x70000000 处的syscall函数.

* --func参数支持c++的类函数符号,也就是在解析elf的调试符号要先Demangled再用strcmp进行比较?

* 是否需要实现一个控制台来交互控制被跟踪的程序?

* 是否需要重用生成的 coredump 文件?

* 统计所有系统调用原型,如果某个系统调用的参数是内存地址，自动转储这个内存地址指向的内存页.
  这里有2个坑:
  1. 某些系统调用有几个参数，第一个参数决定后续参数的数据类型，比如:
       ioctl, pctrl, ptrace, sysfs.
  2. libc封装的系统调用的参数顺序跟libc实际传给内核接收后的参数顺序是不一样的.
     比如 clone 系统调用，libc的封装原型是第三个参数是 flags，但是传给内核时候把第一个参数
     当做 flags 参数.
     gcc的用户态参数传递依次是: rdi, rsi, rdx, rcx, r8, r9;
     内核读取用户态参数依次是: rdi, rsi, rdx, r10, r8 r9.
     libc的函数原型: `int clone(int fn, void *child_stack, int flags, void *arg, ...)`
    
汇编代码:
```
   0x00007ffff7906960 < +0>:	mov    rax,0xffffffffffffffea
   0x00007ffff7906967 < +7>:	test   rdi,rdi
   0x00007ffff790696a <+10>:	je     0x7ffff79069c7 <clone+103>
   0x00007ffff790696c <+12>:	test   rsi,rsi
   0x00007ffff790696f <+15>:	je     0x7ffff79069c7 <clone+103>
   0x00007ffff7906971 <+17>:	sub    rsi,0x10
   0x00007ffff7906975 <+21>:	mov    QWORD PTR [rsi+0x8],rcx
   0x00007ffff7906979 <+25>:	mov    QWORD PTR [rsi],rdi
   0x00007ffff790697c <+28>:	mov    rdi,rdx                  ;a3->a1
   0x00007ffff790697f <+31>:	mov    rdx,r8
   0x00007ffff7906982 <+34>:	mov    r8,r9
   0x00007ffff7906985 <+37>:	mov    r10,QWORD PTR [rsp+0x8]
   0x00007ffff790698a <+42>:	mov    eax,0x38                 ;--> sys_clone
   0x00007ffff790698f <+47>:	syscall
```


* 如何验证tls是否正确?  gdb没有查看tls的命令,也没有权限访问gdt/ldt.
  只能在插件里面提供一个菜单，单击这个菜单就查看当前事件的tls的16进制数据.

* strace 定义 USE_LIBUNWIND 编译宏，然后使用-k选项即可打印 stack trace，.
  可以用来对比我们的strace2的准确性和性能.

* 如何更聪明些, 识别到匿名的rw内存段是否哪个模块申请的?局部相关性原理,在加载一个具体so后连
    续的几次匿名rw内存段就是这个模块申请的?

* 对于qt这种大量使用 new 关键字动态创建的类实例或类成员变量，如何识别?
  开发人员在代码里面导出一些全局变量指针,在这些全局变量指针里面保存这些对象的内存地址.
  利用局部相关性.

* 如何知道某个malloc/alloc/calloc/realloc 是某个模块的代码发起的?
  使用 preload hook libc的malloc/alloc/calloc/realloc 多大作用?
  某些函数的malloc和free非常频繁?

* 如何dump一个全局list对象, map对象, vector对象?
  objdump --dwarf=info xxx.exe,看起来通过dwarf文件是可以知道任意类型的成员布局的.

* 是否需要添加一个配置项目: cpu affinity 把多线程程序限定在指定单核上运行.

* 申威上查看 coredump 文件有这个警告重要吗?
    warning: Unexpected size of section `.reg/8881' in core file.
    Core was generated by `/ls'.

* 查看事件的时候，如果事件发生时刻记录了多个线程，默认显示的是主线程，需要切换一下线程才行.

* 国产平台上如果发生了 syscall exit missing 需要 dump 一个模拟的 -EINTER 的 syscall-exit 事件.
