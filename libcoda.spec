Summary: 	coda developement library
Name: 		libcoda-devel
Version: 	0.1.12
Release: 	0%{?dist}
License: 	LGPL
Source: 	libcoda-%{version}.tar.gz
Group:		Developement/Libraries
BuildRoot: 	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	expat-devel cmake gcc-c++

%description
Coda developement library: different tools from Bachan.

%prep
%setup -q -n libcoda-%{version}

%build
cmake -D SKIP_RELINK_RPATH=ON . -DCMAKE_INSTALL_PREFIX=/usr -DCFLAGS="${CFLAGS}" -DCXXFLAGS="${CXXFLAGS}"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir %{buildroot}
make DESTDIR=%{buildroot} install
mkdir $RPM_BUILD_ROOT/usr/share

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/coda_url_parser
%{_includedir}/coda
%{_libdir}/libcoda.a

%changelog
* Sat Mar 09 2013 Alexander Pankov <pianist@usrsrc.ru> - 0.1.0
+ First RPM package

