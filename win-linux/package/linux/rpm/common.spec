Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
Summary: Office suite
License: AGPLv3
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} %{_support_mail}
AutoReq: no
AutoProv: no

%description
%{_company_name} %{_product_name} is a free office suite that combines text,
spreadsheet and presentation editors allowing to create, view and edit
documents without an Internet connection.
It is fully compatible with Office Open XML formats: .docx, .xlsx, .pptx.

%if "%{_company_name}" == "ONLYOFFICE"
%package help
Summary: Offline help for %{_company_name} %{_product_name}
BuildArch: noarch
Requires: %{_package_name}

%description help
This package contains offline help files.
%endif

%prep
rm -rf "%{buildroot}"

%build

%install
BUILD_DIR=../../../build
BIN_DIR=%{buildroot}%{_bindir}
DATA_DIR=%{buildroot}%{_datadir}
OPT_DIR=%{buildroot}/opt

mkdir -p %{buildroot}
cp -rt %{buildroot}/ $BUILD_DIR/main/*
echo "package = rpm" > $OPT_DIR/%{_desktopeditors_prefix}/converter/package.config

%if "%{_company_name}" == "ONLYOFFICE"
ln -srf $BIN_DIR/%{_desktopeditors_exec} $BIN_DIR/desktopeditors
cp -rt %{buildroot}/ $BUILD_DIR/help/*
%else
ETC_DIR=%{buildroot}%{_sysconfdir}
mkdir -p $ETC_DIR/%{_package_name}
%endif

%clean
rm -rf "%{buildroot}"

%files
%attr(-, root, root) /opt/*
%attr(-, root, root) %{_datadir}/applications/*
%attr(-, root, root) %{_datadir}/doc/*
%attr(-, root, root) %{_datadir}/icons/*
%attr(-, root, root) %{_datadir}/licenses/*
%attr(-, root, root) %{_datadir}/mime/*
%attr(755, root, root) %{_bindir}/%{_desktopeditors_exec}
%if "%{_company_name}" == "ONLYOFFICE"
%attr(-, root, root) %{_bindir}/desktopeditors
%exclude /opt/%{_desktopeditors_prefix}/editors/web-apps/apps/*/main/resources/help
%else
%attr(755, root, root) %{_bindir}/%{_imageviewer_exec}
%attr(755, root, root) %{_bindir}/%{_videoplayer_exec}
%attr(-, root, root) %{_bindir}/%{_package_name}
%attr(777, root, root) %{_sysconfdir}/%{_package_name}
%endif

%if "%{_company_name}" == "ONLYOFFICE"
%files help
%defattr(-, root, root, -)
/opt/%{_desktopeditors_prefix}/editors/web-apps/apps/*/main/resources/help
%endif

%changelog

%include ../build/main/usr/share/doc/%{_package_name}/ChangeLog
