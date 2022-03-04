Summary: Desktop editors for text, spreadsheet and presentation files
Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
License: AGPLv3
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} %{_support_mail}
BuildArch: %{_package_arch}
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
%attr(-, root, root) %{_datadir}/applications/*
%attr(755, root, root) %{_bindir}/%{_desktopeditors_exec}
%if "%{_company_name}" == "ONLYOFFICE"
%attr(-, root, root) %{_bindir}/desktopeditors
%else
%attr(755, root, root) %{_bindir}/%{_imageviewer_exec}
%attr(755, root, root) %{_bindir}/%{_videoplayer_exec}
%attr(-, root, root) %{_bindir}/%{_package_name}
%attr(777, root, root) %{_sysconfdir}/%{_package_name}
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
  "$XDG_ICON_RESOURCE" install --size "${size%.png}" "$icon" "%{_package_name}"
done

UPDATE_MENUS="`which update-menus 2> /dev/null || true`"
if [ -x "$UPDATE_MENUS" ]; then
  update-menus
fi

MIMEAPPS_LIST="/usr/share/applications/mimeapps.list"
if [ ! -f "$MIMEAPPS_LIST" ]; then
  echo "[Default Applications]" >"$MIMEAPPS_LIST"
fi
if [ $(cat "$MIMEAPPS_LIST" | grep x-scheme-handler/oo-office | wc -l) -eq "0" ]; then
  echo "x-scheme-handler/oo-office=%{_desktopeditors_exec}.desktop" >>"$MIMEAPPS_LIST"
fi
if [ $(cat "$MIMEAPPS_LIST" | grep text/docxf | wc -l) -eq "0" ]; then
  echo "text/docxf=%{_desktopeditors_exec}.desktop" >>"$MIMEAPPS_LIST"
fi
if [ $(cat "$MIMEAPPS_LIST" | grep text/oform | wc -l) -eq "0" ]; then
  echo "text/oform=%{_desktopeditors_exec}.desktop" >>"$MIMEAPPS_LIST"
fi

xdg-mime install --mode system /opt/%{_desktopeditors_prefix}/mimetypes/onlyoffice-docxf.xml
xdg-mime install --mode system /opt/%{_desktopeditors_prefix}/mimetypes/onlyoffice-oform.xml

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
XDG_ICON_RESOURCE="`which xdg-icon-resource 2> /dev/null || true`"
if [ ! -x "$XDG_ICON_RESOURCE" ]; then
  echo "Error: Could not find xdg-icon-resource" >&2
  exit 1
fi
for icon in "/opt/%{_desktopeditors_prefix}/asc-de-"*.png; do
  size="${icon##*/asc-de-}"
  "$XDG_ICON_RESOURCE" uninstall --size "${size%.png}" "%{_package_name}"
done

UPDATE_MENUS="`which update-menus 2> /dev/null || true`"
if [ -x "$UPDATE_MENUS" ]; then
  update-menus
fi

# Update cache of .desktop file MIME types. Non-fatal since it's just a cache.
#update-desktop-database > /dev/null 2>&1 || true

%postun

set -e 		# fail on any error
