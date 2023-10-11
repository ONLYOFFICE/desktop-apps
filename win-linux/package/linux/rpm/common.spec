Summary: Desktop editors for text, spreadsheet and presentation files
Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
License: AGPLv3
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} %{_support_mail}
AutoReq: no
AutoProv: no

%description
%{_company_name} %{_product_name} installation package
 %{_company_name} %{_product_name} is an application for editing office documents (text documents, spreadsheets and presentations) from %{_company_name} cloud portal on local computer without browser using.

%if "%{_company_name}" == "ONLYOFFICE"
%package help
Summary: Desktop editors local help files
BuildArch: noarch
Requires: %{_package_name}

%description help
%{_company_name} %{_product_name} local help files
 %{_company_name} %{_product_name} is an application for editing office documents (text documents, spreadsheets and presentations) from %{_company_name} cloud portal on local computer without browser using.
 This package contains the local help files.
%endif

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
echo "package = rpm" > $DESKTOPEDITORS_PREFIX/converter/package.config
mkdir -p $DATA_DIR/doc/%{_package_name}
cp $COMMON/usr/share/doc/%{_package_name}/NEWS $DATA_DIR/doc/%{_package_name}

%if "%{_company_name}" == "ONLYOFFICE"
# help
cp -r $COMMON/help/desktopeditors/* $DESKTOPEDITORS_PREFIX/

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
%attr(-, root, root) %{_datadir}/applications/*
%attr(-, root, root) %{_datadir}/doc/*
%attr(755, root, root) %{_bindir}/%{_desktopeditors_exec}
%if "%{_company_name}" == "ONLYOFFICE"
%attr(-, root, root) %{_bindir}/desktopeditors
%exclude /opt/%{_desktopeditors_prefix}/editors/web-apps/apps/common/main/resources/help
%exclude /opt/%{_desktopeditors_prefix}/editors/web-apps/apps/documenteditor/main/resources/help
%exclude /opt/%{_desktopeditors_prefix}/editors/web-apps/apps/presentationeditor/main/resources/help
%exclude /opt/%{_desktopeditors_prefix}/editors/web-apps/apps/spreadsheeteditor/main/resources/help
%else
%attr(755, root, root) %{_bindir}/%{_imageviewer_exec}
%attr(755, root, root) %{_bindir}/%{_videoplayer_exec}
%attr(-, root, root) %{_bindir}/%{_package_name}
%attr(777, root, root) %{_sysconfdir}/%{_package_name}
%endif

%if "%{_company_name}" == "ONLYOFFICE"
%files help
%defattr(-, root, root, -)
/opt/%{_desktopeditors_prefix}/editors/web-apps/apps/common/main/resources/help
/opt/%{_desktopeditors_prefix}/editors/web-apps/apps/documenteditor/main/resources/help
/opt/%{_desktopeditors_prefix}/editors/web-apps/apps/presentationeditor/main/resources/help
/opt/%{_desktopeditors_prefix}/editors/web-apps/apps/spreadsheeteditor/main/resources/help
%endif

%pre

%post

set -e 		# fail on any error
set -u 		# treat unset variable as errors

# Add icons to the system icons
XDG_ICON_RESOURCE="`which xdg-icon-resource 2> /dev/null || true`"
if [ ! -x "$XDG_ICON_RESOURCE" ]; then
  echo "Error: Could not find xdg-icon-resource" >&2
  exit 1
fi
for icon in "/opt/%{_desktopeditors_prefix}/asc-de-"*.png; do
  size="${icon##*/asc-de-}"
  if [ $1 == 2 ];then #upgrade (not install)
    "$XDG_ICON_RESOURCE" uninstall --size "${size%.png}" "%{_package_name}"
  fi
  "$XDG_ICON_RESOURCE" install --size "${size%.png}" "$icon" "%{_package_name}"
done

UPDATE_MENUS="`which update-menus 2> /dev/null || true`"
if [ -x "$UPDATE_MENUS" ]; then
  update-menus
fi

xdg-mime install --mode system /opt/%{_desktopeditors_prefix}/mimetypes/onlyoffice-docxf.xml
xdg-mime install --mode system /opt/%{_desktopeditors_prefix}/mimetypes/onlyoffice-oform.xml

update-mime-database /usr/share/mime
update-desktop-database /usr/share/applications

# Update cache of .desktop file MIME types. Non-fatal since it's just a cache.
#update-desktop-database > /dev/null 2>&1 || true

%preun

set -e

action="$1"
if [ "$2" = "in-favour" ]; then
  # Treat conflict remove as an upgrade.
  action="upgrade"
fi
# Don't clean-up just for an upgrade.`
if [ "$action" = "upgrade" ] ; then
  exit 0
fi

# Remove icons from the system icons
if [ $1 == 0 ];then #uninstall (not upgrade)
  XDG_ICON_RESOURCE="`which xdg-icon-resource 2> /dev/null || true`"
  if [ ! -x "$XDG_ICON_RESOURCE" ]; then
    echo "Error: Could not find xdg-icon-resource" >&2
    exit 1
  fi
  for icon in "/opt/%{_desktopeditors_prefix}/asc-de-"*.png; do
    size="${icon##*/asc-de-}"
    "$XDG_ICON_RESOURCE" uninstall --size "${size%.png}" "%{_package_name}"
  done
fi

UPDATE_MENUS="`which update-menus 2> /dev/null || true`"
if [ -x "$UPDATE_MENUS" ]; then
  update-menus
fi

# Update cache of .desktop file MIME types. Non-fatal since it's just a cache.
#update-desktop-database > /dev/null 2>&1 || true

%postun

set -e 		# fail on any error

%posttrans

#for compatibility with old RPMs
XDG_ICON_RESOURCE="`which xdg-icon-resource 2> /dev/null || true`"
if [ ! -x "$XDG_ICON_RESOURCE" ]; then
  echo "Error: Could not find xdg-icon-resource" >&2
  exit 1
fi
for icon in "/opt/%{_desktopeditors_prefix}/asc-de-"*.png; do
  size="${icon##*/asc-de-}"
  "$XDG_ICON_RESOURCE" install --size "${size%.png}" "$icon" "%{_package_name}"
done

%changelog

%include ../common/usr/share/doc/%{_package_name}/ChangeLog
