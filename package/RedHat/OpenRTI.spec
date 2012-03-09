#define gitdate 205f2905

Summary: HLA/RTI runtime infrastructure
Name: OpenRTI
Version: 0.1
Release: 2%{?dist}
License: LGPL
Group: System Environment/Libraries
#URL: http://www.openrti.org
#Source0: http://www.openrti.org/%{name}-%{version}.tar.bz2
Source0: %{name}-%{version}.tar.bz2
#Source1: make-git-snapshot.sh

Requires: expat
# Requires: python

BuildRequires: cmake
BuildRequires: expat-devel
#BuildRequires: python-devel

%description
Direct Rendering Manager runtime library

%package devel
Summary: HLA/RTI runtime infrastructure development package
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
HLA/RTI runtime infrastructure development package

# %package -n rtinode
# Summary: HLA/RTI server node
# Group: Simulation

# %description -n rtinode
# HLA/RTI server node

%prep
%setup -q -c

%build
mkdir -p BUILD
pushd BUILD
CFLAGS="${RPM_OPT_FLAGS} -pthread"
%cmake ../OpenRTI-%{version}
make VERBOSE=1 %{?_smp_mflags}
popd

%install
pushd BUILD
make install DESTDIR=${RPM_BUILD_ROOT}
rm ${RPM_BUILD_ROOT}/%{_libdir}/libOpenRTI.so
popd

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
#%doc README
%{_bindir}/rtinode
%{_libdir}/libOpenRTI.so.1
%{_libdir}/libOpenRTI.so.1.0.0

%{_libdir}/librti1516.so.1
%{_libdir}/librti1516.so.1.0.0
%{_libdir}/libfedtime1516.so.1
%{_libdir}/libfedtime1516.so.1.0.0

%{_libdir}/libRTI-NG.so.1
%{_libdir}/libRTI-NG.so.1.3.0
%{_libdir}/libFedTime.so.1
%{_libdir}/libFedTime.so.1.3.0

# %files -n rtinode
# %defattr(-,root,root,-)
# %{_bindir}/rtinode

%files devel
%defattr(-,root,root,-)
# %{_libdir}/libOpenRTI.so
# The RTI13 development files
%{_includedir}/federateAmbServices.hh
%{_includedir}/RTI.hh
%{_includedir}/baseTypes.hh
%{_includedir}/RTItypes.hh
%{_includedir}/fedtime.hh
%{_includedir}/RTIambServices.hh
%{_includedir}/NullFederateAmbassador.hh
%{_libdir}/libRTI-NG.so
%{_libdir}/libFedTime.so

# The RTI1516 development files
%{_includedir}/rti1516
%{_libdir}/libfedtime1516.so
%{_libdir}/librti1516.so


#%changelog
