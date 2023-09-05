# deepin-unioncode

[![en](https://img.shields.io/badge/lang-en-red.svg)](./README.en.md)

深度集成开发环境是由深度公司自主研发，具有多语言、跨平台兼容特性的轻量级集成开发环境。

## 构建依赖

_当前的开发分支为**master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

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

## 安装

1. 确保已经安装所有依赖库。


``` shell
$ git clone https://github.com/linuxdeepin/deepin-unioncode.git 
(gitee: $ git clone https://gitee.com/deepin-community/deepin-unioncode.git)
$ cd deepin-unioncode
$ sudo apt build-dep ./
```

2. 构建:

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```

3. 安装:

```shell
$ sudo cmake --build build --target install
```

可执行程序为 `/usr/bin/deepin-unioncode`

## 使用

启动栏执行 `deepin-unioncode`

*注意：deepin-unioncode安装后，部分功能需要安装依赖包后才能正常使用。*

## 支持的语言工程

| 语言       | 工程类型      |
| ---------- | ------------- |
| C/C++      | CMake         |
| Java       | Gradle、Maven |
| Python     | 工程目录      |
| JavaScript | 工程目录      |

## 开发应用列举

| 应用名              | 描述         | 仓库地址                                                     |
| ------------------- | ------------ | ------------------------------------------------------------ |
| deepin-draw         | 深度画板应用 | https://github.com/linuxdeepin/deepin-draw<br />(https://gitee.com/deepin-community/deepin-draw.git) |
| deepin-image-viewer | 深度看图应用 | https://github.com/linuxdeepin/deepin-image-viewer<br />(https://gitee.com/deepin-community/deepin-image-viewer.git) |
| deepin-album        | 深度相册应用 | https://github.com/linuxdeepin/deepin-album<br />(https://gitee.com/deepin-community/deepin-album.git) |

## 帮助

- [官方论坛](https://bbs.deepin.org/) 
- [用户使用手册](https://wiki.deepin.org/zh/05_HOW-TO/02_%E5%BC%80%E5%8F%91%E7%9B%B8%E5%85%B3/deepin-unioncode)
- [开发者中心](https://github.com/linuxdeepin/developer-center) 
- [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
- [聊天室](https://webchat.freenode.net/?channels=deepin)
- [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

deepin-unioncode 在 [GPL-3.0-or-later](LICENSE.txt)下发布。

## 三方库支持

| 序号 | 三方库名称    | 版本号     | 引用方式（动态、静态、源码） | 三方库协议类型 |
| ---- | ------------- | ---------- | ---------------------------- | -------------- |
| 1    | cppdap        | NA         | 源码                         | Apache 2.0     |
| 2    | googletest    | 1.11.0     | 静态                         | BSD3           |
| 3    | nlohmann/json | 3.10.4     | 静态                         | MIT            |
| 4    | scintilla     | 5.15       | 源码                         | HPND           |
| 5    | lexilla       | 5.14       | 源码                         | HPND           |
| 6    | marl          | NA         | 静态                         | Apache 2.0     |
| 7    | qtermwidget   | 0.14.1     | 源码                         | GPLV2+ BSD3    |
| 8    | GitQlient     | tag/v1.4.3 | 源码                         | GPLV-2.1+      |
| 9    | libclang/     | > 1:7~     | 动态                         | Apache  2.0    |
| 10   | llvm          | > 1:7~     | 动态                         | Apache  2.0    |
| 11   | json-rpc-cpp  | 1.4.1      | 源码                         | MIT            |

## 界面预览
- 语言支持

  ![](./docs/rc/language-support.png)

- 代码迁移

  ![code porting](./docs/rc/code-porting.png)

- 调试模式

  ![](./docs/rc/debug-mode.png)
  
- 反向调试

  ![reverse debug](./docs/rc/reverse-debug.png)
  
- 版本管理

  ![](./docs/rc/version-management.png)
