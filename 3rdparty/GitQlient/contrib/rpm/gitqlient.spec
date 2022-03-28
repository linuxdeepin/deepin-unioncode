Name:    {{{ git_name name="gitqlient" }}}
Version: 1.4.3
Release: {{{ git_version }}}%{?dist}
Summary: A multi-platform Git client

License:    LGPLv2
URL:        https://github.com/francescmm/GitQlient
VCS:        {{{ git_vcs }}}

Source:     {{{ git_pack }}}

BuildRequires:  /usr/bin/qmake-qt5
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5WebEngineWidgets)
BuildRequires:  pkgconfig(Qt5WebChannel)

BuildRequires: desktop-file-utils

%description
%{summary}.

%prep
{{{ git_setup_macro }}}

%build
%if 0%{?suse_version}
#https://en.opensuse.org/openSUSE:Build_system_recipes#qmake
qmake-qt5 -makefile \
   QMAKE_CFLAGS+="%optflags" \
   QMAKE_CXXFLAGS+="%optflags" \
   QMAKE_STRIP="/bin/true" \
   PREFIX=%{_prefix} \
   GitQlient.pro
%else
%qmake_qt5 PREFIX=%{_prefix} GitQlient.pro
%endif

%make_build

%install
make install INSTALL_ROOT=%{buildroot}

desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop

%files
%doc README.md
%license LICENSE
%{_bindir}/gitqlient
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.*

%changelog
{{{ git_changelog }}}
