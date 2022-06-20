Summary: Desktop editors for text, spreadsheet and presentation files
Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
License: AGPLv3
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} <%{_support_mail}>
AutoReq: no
AutoProv: no

%description
%{_company_name} %{_product_name} installation package
 %{_company_name} %{_product_name} is an application for editing office documents (text documents, spreadsheets and presentations) from %{_company_name} cloud portal on local computer without browser using.

%prep
rm -rf "%{buildroot}"

%build

%install
COMMON=%{_builddir}/../../../common

BIN_DIR=%{buildroot}%{_bindir}
DATA_DIR=%{buildroot}%{_datadir}

DESKTOPEDITORS_PREFIX=%{buildroot}/opt/%{_desktopeditors_prefix}
mkdir -p $BIN_DIR $DATA_DIR/applications $DESKTOPEDITORS_PREFIX

cp -r $COMMON/opt/desktopeditors/* $DESKTOPEDITORS_PREFIX
cp -t $BIN_DIR $COMMON/usr/bin/%{_desktopeditors_exec}
cp -t $DATA_DIR/applications $COMMON/usr/share/applications/%{_desktopeditors_exec}.desktop
cp -r $COMMON/usr/share/mime $DATA_DIR
cp -r $COMMON/usr/share/icons $DATA_DIR

%if "%{_company_name}" == "ONLYOFFICE"
ln -srf $BIN_DIR/%{_desktopeditors_exec} $BIN_DIR/desktopeditors
%else
ETC_DIR=%{buildroot}%{_sysconfdir}
mkdir -p $ETC_DIR/%{_package_name}

MEDIAVIEWER_PREFIX=%{buildroot}/opt/%{_mediaviewer_prefix}
mkdir -p $MEDIAVIEWER_PREFIX
cp -r $COMMON/opt/mediaviewer/* $MEDIAVIEWER_PREFIX/
cp -t $BIN_DIR \
  $COMMON/usr/bin/%{_imageviewer_exec} \
  $COMMON/usr/bin/%{_videoplayer_exec}
cp -t $DATA_DIR/applications \
  $COMMON/usr/share/applications/%{_imageviewer_exec}.desktop \
  $COMMON/usr/share/applications/%{_videoplayer_exec}.desktop
ln -srf $BIN_DIR/%{_desktopeditors_exec} $BIN_DIR/%{_package_name}
%endif

%clean
rm -rf "%{buildroot}"

%files
%attr(-, root, root) /opt/*
%attr(644, root, root) %{_datadir}/applications/*.desktop
%attr(644, root, root) %{_datadir}/share/mime/packages/*.xml
%attr(644, root, root) %{_datadir}/share/icons/hicolor/*/apps/*.png
%attr(755, root, root) %{_bindir}/%{_desktopeditors_exec}
%if "%{_company_name}" == "ONLYOFFICE"
%attr(-, root, root) %{_bindir}/desktopeditors
%else
%attr(755, root, root) %{_bindir}/%{_imageviewer_exec}
%attr(755, root, root) %{_bindir}/%{_videoplayer_exec}
%attr(-, root, root) %{_bindir}/%{_package_name}
%attr(777, root, root) %{_sysconfdir}/%{_package_name}
%endif

