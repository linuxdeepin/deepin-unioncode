![GitQlient logo](https://github.com/francescmm/GitQlient/blob/master/src/resources/icons/GitQlientLogo96.png "GitQlient")

# GitQlient: Multi-platform Git client written with Qt

[![Linux](https://github.com/francescmm/GitQlient/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/francescmm/GitQlient/actions/workflows/ubuntu.yml)
[![MacOS](https://github.com/francescmm/GitQlient/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/francescmm/GitQlient/actions/workflows/macos.yml)
<a href="https://ci.appveyor.com/project/francescmm/gitqlient/branch/master" target="_blank"><img src="https://ci.appveyor.com/api/projects/status/ihw50uwdiim952c0/branch/master"/></a>

GitQlient, pronounced as git+client (/gɪtˈklaɪənt/) is a multi-platform Git client originally forked from QGit. Nowadays it goes beyond of just a fork and adds a lot of new functionality.

![GitQlient main screen](/docs/assets/GitQlient.png)

## Main features

Some of the major feature you can find are:

1. Easy access to remote actions like: push, pull, submodules management and branches
2. Branches management
3. Tags and stashes management
4. Submodules handling
5. Allow to open several repositories in the same window
6. Better visualization of the commits and the work in progress
7. Better visualization of the repository view
8. GitHub/GitLab integration
9. Embedded text editor with syntax highlight for C++

For all the features take a look to the [Release Notes in the Wiki](https://github.com/francescmm/GitQlient/wiki).

## User Manual

Please, if you have any doubts about how to use it or you just want to know all you can do with GitQlient, take a look to [the user manual in here](https://francescmm.github.io/GitQlient).

It is planned to release for Linux, MacOs and Windows. However, take into account that the development environment is based on Linux and it will be the first platform released.

## How to install GitQlient

### Linux

#### AppImage

GitQlient is offered in the form of AppImage. Although a big binary, it includes all the libraries needed to run. It is a portable multi-OS version so just download the versio you want from the [releases page](https://github.com/francescmm/GitQlient/releases) and execute it (make sure it has execution rights).

#### RPM package for Fedora

Package [available](https://src.fedoraproject.org/rpms/gitqlient) in official Fedora repos. To install, type the following command in the terminal:

```
sudo dnf install gitqlient
```

#### DEB package for Ubuntu

For now there is no GitQlient in the official repos of Ubuntu or Debian. However, to install it via .deb package, go to the [releases page](https://github.com/francescmm/GitQlient/releases) and download the package from the version you want. Once you download it, you can install it using apt-get command as root.

**Note:** This option is available only from GitQlient 1.4.3.

### Windows

For Windows 10 there is a package provided on every release. It installs all the dependencies and creates an shortcut in the Desktop. From the [releases page](https://github.com/francescmm/GitQlient/releases), download the GitQlientInstaller-<version>.exe binary from the release you want to use.

### MacOS

For MacOS there is also a DMG package you can install as usual. Take into account that because of lack of resources this version is not tested.

## Translating GitQlient

GitQlient is using the translation system of Qt. That means that for every new language two files are needed: .ts and .qm. The first one is the text translation and the second one is a compiled file that GitQlient will load.

To add a new translation, please generate those files and add them to the resources.qrc.

For more information on [Qt translation system](https://doc.qt.io/qt-5/linguist-manager.html).

### Building GitQlient

In the [User Manual](https://francescmm.github.io/GitQlient/#appendix-b-build) you can find a whole section about building GitQlient and what dependencies you need.
  
## Licenses

Most of the icons on GitQlient are from Font Awesome. [The license states is GPL friendly](https://fontawesome.com/license/free). Those icons that are not from Font Awesome are custom made icons.

The font used bt GitQlient is DejaVu Sans and DejaVu Sans Mono. It is a free font used by most of the Linux distros and [its license can be found on GitHub](https://github.com/dejavu-fonts/dejavu-fonts/blob/master/LICENSE).
