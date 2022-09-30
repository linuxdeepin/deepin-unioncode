## 每修改一次代码必须进行以下回归测试
1. 必须用fast和hard模式测试 gedit, deepin-music, testclone, testfork的记录和回放.
2. hard模式还要测试一下 --func=main.

```
#!/bin/bash
./emd --mode=$1 --func=$2 ./testclone ~/.config/emd.json

#!/bin/bash
./emd --mode=$1 --func=$2 ./testfork ./testclone

#!/bin/bash
./emd --mode=$1 --func=$2 /usr/bin/gedit

#!/bin/bash
./emd --mode=$1 --func=$2 /home/usr/Downloads/dumper/deepin-music-3.1.8.1/dist/bin/deepin-music
```
3. 事件回放的时候，至少选中３个事件(中间，头，尾部)的 call-frame back-trace.
## 如何查看进程的maps?
1. 在emd控制台输入 log 1开启日志模式
2. 再次查看改事件即可看到: MD_LINUX_MAPS开头的日志就是当前进程的maps文件

## 记录失败排查方法

## 禁用转储堆栈,全局变量,可写内存段,系统调用参数指向的内存块,maps等
max_syscall_parameter_size 和 stacks.max_size 的值都设为0. 

### hard 模式
1. 开启debug日志并且写stdout.
2. 在终端里面重定向stdout到文件, 记录完成后搜索ERROR,WARNING日志.

### fast 模式
1. 开启debug日志,写stdout.
2. 定义环境变量 ST2_DEBUG_SYSCALL
3. 在终端里面重定向stdout到文件, 记录完成后搜索ERROR,WARNING日志.
4. 如果是崩溃，replay模式控制台用sig命令搜索信号.
5. 还有一种情况是在读取系统调用参数是内存指针时候指向的内存时候，内存地址有问题导致越界访
   问; emd replay 看看是那个系统调用触发的，禁止该系统调用即可,有些系统调用比如munmap,
   mprotect, shmdt执行之后就释放内存了，再读取肯定会崩溃.

#### 启动时候死锁
禁止dump任何数据看看.

1. gab attach emd
2. 在gdb控制台输入 p do_pause(pid)暂停线程 
3. 在gdb控制台输入 p do_bt(pid) 看看每个线程停止哪里了?
   定位到源码:objdump -d xxx.so得到函数的原始起始地址base,
   然后用addr2line -fe xxx.so base+0x????
4. 在gdb控制台输入 p do_play(pid)继续运行线程 
5. 读取内存数据, cat /proc/pid/maps,然后用dd读取进程数据.
   dd if=/proc/pid/mem of=tmp.bin bs=1 skip=$((0x7fe6967b8000)) count=100
6. 反汇编: 先dd然后objdump

## libsyscallpreload 编译debug方法:
修改 cmake-build-dir/CMakeFiles/syscall_intercept_base_c.dir/flags.make:
为如下编译配置:
```
C_FLAGS =  -Werror -Wall -Wextra -pedantic -Wno-c90-c99-compat -g -O0 -fPIC
    -fvisibility=default   -I/usr/include/capstone -std=c99
```
然后删除编译目录的所有.o文件，重新编译:
`find . -type f -name "*.o" -exec rm -f {} \;`

#### 生成coredump
写一个脚本coredump,保存到/home/user/coredump 
```
#!/bin/bash
cat > /tmp/core
```

然后用root用户运行:
```
chmod +x /home/xxx/coredump 
echo \|/home/xxx/coredump > /proc/sys/kernel/core_pattern
```

#### 记录过程中如何停止记录
pgrep找到被记录的程序的pid,用kill给这个pid发SIGQUIT信号.

#### 回放的时候,寄存器值正常，但是函数栈回溯不正常,是匹配了错误的 maps 了.
此时 event_man.cc的 AddDataToMapping 会打印警告:
```
    LOG(WARNING) << "data mapping is not enough:" << HEX(mapping.start_address)
        << "," << mapping.data.size() << "/" << (mapping.end_address - base);
```
还有一种情况是trace目录的 hardlink 文件不存在或没有权限进行 hardlink 创建无效.
导致 corefile 找不到对应的elf文件.

```
-NeoKylin Desktop
-5.0_U2
-loongson_64
-Release
-B053/20181229
Linux localhost.localdomain 3.10.0 #1 SMP PREEMPT Fri Dec 14 15:20:30 CST 2018 00022-g5b64ed3 mips64
gcc 4.9.3
```

#### 系统调用拦截后导致程序启动崩溃
1. 查看堆栈是否有蛛丝马迹
2. 二分法初步增大拦截的 syscall 指令总数，来确定是第几次的 syscall 拦截导致的.
3. 禁止这个地址的 syscall 拦截.
