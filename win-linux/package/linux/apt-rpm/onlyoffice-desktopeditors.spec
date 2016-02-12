Summary: Desktop editors for text, spreadsheet and presentation files
Name: {{PACKAGE_NAME}}
Version: {{PRODUCT_VERSION}}
Release: {{BUILD_NUMBER}}
License: Commercial
Group: Applications/Office
URL: http://onlyoffice.com/
Vendor: ONLYOFFICE (Online documents editor)
Packager: ONLYOFFICE (Online documents editor) <support@onlyoffice.com>
Requires: libX11, libXScrnSaver, libcurl, libgtkglext, libcairo, fonts-ttf-dejavu
BuildArch: x86_64
AutoReq: no
AutoProv: no

%description
Enter your description

%prep
rm -rf "$RPM_BUILD_ROOT"

%build

%install

#install desktopeditor files
mkdir -p "$RPM_BUILD_ROOT/opt/onlyoffice/desktopeditor"
cp -r ../../../common/desktopeditor/* "$RPM_BUILD_ROOT/opt/onlyoffice/desktopeditor"

mkdir -p "$RPM_BUILD_ROOT/usr/bin"
cp -r ../../../common/bin/* "$RPM_BUILD_ROOT/usr/bin"

mkdir -p "$RPM_BUILD_ROOT/usr/share/applications"
cp -r ../../../common/applications/*.desktop "$RPM_BUILD_ROOT/usr/share/applications"

#install fonts
mkdir -p "$RPM_BUILD_ROOT/usr/share/fonts/truetype/onlyoffice/desktopeditor/"
cp -r ../../../common/fonts/* "$RPM_BUILD_ROOT/usr/share/fonts/truetype/onlyoffice/desktopeditor/"

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%attr(-, root, root) /opt/onlyoffice/desktopeditor/*
%attr(-, root, root) /usr/bin/desktopeditor
%attr(-, root, root) /usr/share/applications/*.desktop
%attr(-, root, root) /usr/share/fonts/truetype/onlyoffice/*

%pre

%post
ln -sf /usr/lib64/libcurl.so.4 /usr/lib64/libcurl-gnutls.so.4

%preun

%postun

%changelog
* Tue Feb 02 2016 ONLYOFFICE (Desktop editor) <support@onlyoffice.com>
- Initial release.
