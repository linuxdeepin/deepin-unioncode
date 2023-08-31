# deepin-unioncode

[![zh](https://img.shields.io/badge/lang-zh-red.svg)](./README.md)

Deepin Union Code is a lightweight integrated development environment independently developed by Deepin Technology, featured with multilingual and cross platform compatibility.

## Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

- dh-systemd
- cmake
- qt5-qmake
- qtbase5-dev
- qttools5-dev
- qttools5-dev-tools
- lxqt-build-tools (>= 0.6.0~)
- libssl-dev
- llvm (>=1:7~)
- libclang-dev (>=1:7~)
- libutf8proc-dev
- libcurl-dev
- libmicrohttpd-dev
- libjsoncpp-dev
- libargtable2-dev
- libhiredis-dev
- catch
- libzstd-dev
- libjson-c-dev
- libelf-dev
- libcapstone-dev
- libunwind-dev
- libelfin-dev
- libdbus-1-dev
- libxi-dev
- qtscript5-dev
- libqt5scripttools5
- clang

Installation

## Build from source code

1. Make sure you have installed all dependencies.

``` shell
$ git clone https://github.com/linuxdeepin/deepin-unioncode.git
$ cd deepin-unioncode
$ sudo apt build-dep ./
```

2. Build:

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```

3. Install:

```shell
$ sudo cmake --build build --target install
```

The executable binary file could be found at `/usr/bin/deepin-unioncode`

## Usage

Execute `deepin-unioncode`

*Attention:*

*After the installation of deepin-unicode, some features require the installation of dependency packages before they can be used normally. This version is the root of debian10, which is consistent with the professional version. If you encounter any dependencies, you can directly download them at [[Dependency Package Download Address]](https://community-packages.deepin.com/deepin/pool/main/l/llvm-toolchain-13/)Find the corresponding package for installation in.*

## Supported Language Project

| Language   | Project Type      |
| ---------- | ----------------- |
| C/C++      | CMake             |
| Java       | Gradle、Maven     |
| Python     | Project Direcotry |
| JavaScript | Project Directory |

## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

deepin-unioncode is licensed under [GPL-3.0-or-later](LICENSE)

## 3rdparty support

| 序号 | 使用三方库名称 | 版本号     | 引用方式（动态、静态、源码） | 三方库协议类型 |
| ---- | -------------- | ---------- | ---------------------------- | -------------- |
| 1    | cppdap         | NA         | 静态                         | Apache 2.0     |
| 2    | googletest     | 1.11.0     | 静态                         | BSD3           |
| 3    | nlohmann/json  | 3.10.4     | 静态                         | MIT            |
| 4    | scintilla      | 5.15       | 源码                         | HPND           |
| 5    | lexilla        | 5.14       | 源码                         | HPND           |
| 6    | marl           | NA         | 静态                         | Apache 2.0     |
| 7    | qtremwidget    | 0.14.1     | 源码                         | GPLV2+ BSD3    |
| 8    | GitQlient      | tag/v1.4.3 | 源码                         | GPLV2.1+       |
| 9    | libclang/      | > 1:7~     | 动态                         | Apache  2.0    |
| 10   | llvm           | > 1:7~     | 动态                         | Apache  2.0    |
| 11   | json-rpc-cpp   | 1.4.1      | 源码                         | MIT            |


## Preview

- language support

  ![](./docs/rc/language-support.png)

- code porting

  ![code porting](./docs/rc/code-porting.png)

- debug mode

  ![](./docs/rc/debug-mode.png)
  
- reverse debug

  ![reverse debug](./docs/rc/reverse-debug.png)
  
- version management![](./docs/rc/version-management.png)
