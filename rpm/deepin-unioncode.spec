%global cxxflag -std=c++17
%define _libdir /usr/lib

Name: deepin-unioncode
Version: 1.1.18.1
Release: 1%{?dist}
Summary: IDE 'Deepin Union Code'
License: GPLv3+
URL: http://www.deepin.org
Source0: %{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: clang
BuildRequires: clang-devel
BuildRequires: lxqt-build-tools
BuildRequires: elfutils
BuildRequires: libdwarf
BuildRequires: doxygen
BuildRequires: jsoncpp-devel
BuildRequires: capstone-devel
BuildRequires: llvm-devel
BuildRequires: qt5-linguist
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5OpenGL)
BuildRequires:  pkgconfig(Qt5PrintSupport)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(Qt5Script)


%description
Lightweight cross-platform integrated development environment

%package devel
Summary: IDE 'Union Code' devel library
Requires: %{name} = %{version}-%{release}

%description devel
IDE 'Union Code' development can be carried out through this library

%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
export CXXFLAGS=%{cxxflag}
mkdir build && pushd build
cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_MODULE=ON ../ CMakeLists.txt
%make_build %{?_smp_mflags}
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%license LICENSE
%doc README.md
%{_bindir}/deepin-unioncode
%{_libdir}/%name/*.so
%{_libdir}/%name/plugins/*.so
%{_libdir}/%name/scripts/**
%{_libdir}/%name/tools/**
%{_datadir}/applications/unioncode.desktop
%{_datadir}/%name/configures/*.support
%{_datadir}/%name/configures/icons/*.png
%{_datadir}/%name/translations/*.qm
%{_datadir}/%name/templates/**
%{_datadir}/%name/unioncode-qtermwidget/**
%exclude /usr/include/deepin-unioncode/framework/*

%changelog
* Tue Jan 01 2019 Deepin Packages Builder <packages@linuxdeepin.com> 1.0.0-1
- Initial RPM release.
