Summary: ONLYOFFICE Desktop editors addon
Name: onlyoffice-desktopeditors-addon
Version: {{PRODUCT_VERSION}}
Release: {{BUILD_NUMBER}}
License: Commercial
Group: Applications/Office
URL: http://onlyoffice.com/
Vendor: Ascensio System SIA
Packager: Ascensio System SIA <support@onlyoffice.com>
BuildArch: x86_64
AutoReq: no
AutoProv: no

%description
ONLYOFFICE DesktopEditors addon installation package

%prep
rm -rf "$RPM_BUILD_ROOT"

%build

%install

#install desktopeditor files
LIB_X64=/lib/x86_64-linux-gnu
USR_LIB_X64=/usr/lib/x86_64-linux-gnu
mkdir -p "$RPM_BUILD_ROOT/onlyoffice/desktopeditors/"
cp -t "$RPM_BUILD_ROOT/onlyoffice/desktopeditors/" \
  $USR_LIB_X64/libffi.so.6 \
  $USR_LIB_X64/libgio-2.0.so.0 \
  $LIB_X64/libglib-2.0.so.0 \
  $USR_LIB_X64/libgmodule-2.0.so.0 \
  $USR_LIB_X64/libgobject-2.0.so.0 \
  $LIB_X64/libpcre.so.3 \
  $USR_LIB_X64/libstdc++.so.6

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%attr(-, root, root) /opt/onlyoffice/desktopeditors/*

%pre

%post

%preun

%postun
