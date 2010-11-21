%define base_version		1.4.0.2
%define rel			1

Summary: Program for extracting the data from scanned graphs
Name: g3data
Version: %{base_version}
Release: %{rel}
License: GPL
Group: Applications/Engineering
URL: http://www.acclab.helsinki.fi/~frantz/software/g3data.php
Source0: http://www.acclab.helsinki.fi/~frantz/software/g3data-%{version}.tar.gz
BuildRoot: %{_tmppath}/g3data-%{version}-root

Requires: gtk2

BuildRequires: gtk2-devel
BuildRequires: pkgconfig

%description
g3data is used for extracting data from graphs. In publications
graphs often are included, but the actual data is missing, g3data
makes this extracting process much easier.

%prep
%setup -q -n g3data-%{version}

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man1

%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)

/usr/bin/g3data
/usr/share/man/man1/g3data.1.gz

%doc README.TEST README.SOURCE README.INSTALL test1.gif test1.values test2.gif test2.values gpl.txt

%changelog
* Sun May 1 2005 Jonas Frantz <jonas.frantz@welho.com>
- Fixed bug with logarithmic y-scale
* Sat Aug 10 2003 Jonas Frantz <jonas.frantz@helsinki.fi>
- Initial build

