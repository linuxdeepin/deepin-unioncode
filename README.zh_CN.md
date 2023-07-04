# deepin-unioncode

深度集成开发环境是由深度公司自主研发，具有多语言、跨平台兼容特性的轻量级集成开发环境。

### 依赖

### 构建依赖

_当前的开发分支为**master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

  -  debhelper (>=9),
  - dh-systemd,
  - cmake,
  -  qt5-qmake,
  -  qtbase5-dev,
  -  qttools5-dev,
  -  qttools5-dev-tools,
  -  lxqt-build-tools (>= 0.6.0~),
  -  libssl-dev,
  -  llvm (>=1:7~),
  -  libclang-dev (>=1:7~),
  -  libutf8proc-dev,
  -  libcurl-dev,
  -  libmicrohttpd-dev,
  -  libjsoncpp-dev,
  -  libargtable2-dev,
  -  libhiredis-dev,
  -  catch,
  -  libzstd-dev,
  -  libjson-c-dev,
  -  libelf-dev,
  -  libcapstone-dev,
  -  libunwind-dev, 
  -  libelfin-dev,
  -  libdbus-1-dev,
  -  libxi-dev,
  -  qtscript5-dev,
  -  libqt5scripttools5

## 安装

### 构建过程

1. 确保已经安装所有依赖库。


``` shell
$ git clone https://github.com/linuxdeepin/deepin-unioncode.git
$ cd unioncode
$ sudo apt build-dep ./
```

2. 构建:

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build
```

3. 安装:

```shell
$ sudo cmake --build build --target install
```

可执行程序为 `/usr/bin/deepin-unioncode`

## 使用

执行 `deepin-unioncode`

*注意：*

*deepin-unioncode安装后，部分功能需要安装依赖包后才能正常使用。该版本为debian10的根，与专业版一致，遇到相关依赖可直接在[依赖包下载地址](https://community-packages.deepin.com/deepin/pool/main/l/llvm-toolchain-13/)中找到相应的包进行安装。*

## 支持的语言工程

| Language     | Project Type    |
| ------------ | ----------------|
| C/C++        | CMake           |
| Java         | Grade           |
| Java         | Maven           |
| Python       | Local Direcotry |
| JavaScript   | Local Directory |

## 帮助

- [官方论坛](https://bbs.deepin.org/) 
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

## 界面展示
- 语言支持

  ![](./docs/rc/language support.png)

- 代码分析

  ![](./docs/rc/code analysis.png)

- 插件管理

  ![](./docs/rc/plugin manager.png)

- 代码迁移

  ![code porting](./docs/rc/code porting.png)

- 反向调试

  ![reverse debug](./docs/rc/reverse debug.png)