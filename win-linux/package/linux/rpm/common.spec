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

DESKTOPEDITORS=%{_builddir}/../../../common/desktopeditors

HOME_DIR=%{buildroot}/opt/%{_desktopeditors_prefix}
BIN_DIR=%{buildroot}%{_bindir}
DATA_DIR=%{buildroot}%{_datadir}

#install desktopeditor files
mkdir -p "$HOME_DIR/"
cp -r $DESKTOPEDITORS/home/* "$HOME_DIR/"

#install documentbuilder bin
mkdir -p "$BIN_DIR/" "$DATA_DIR/applications/"
cp $DESKTOPEDITORS/bin/%{_package_name} "$BIN_DIR/"
cp $DESKTOPEDITORS/share/applications/%{_package_name}.desktop "$DATA_DIR/applications/"

%clean
rm -rf "%{buildroot}"

%files
%attr(-, root, root) /opt/%{_desktopeditors_prefix}/*
%attr(755, root, root) %{_bindir}/%{_package_name}
%attr(-, root, root) %{_datadir}/applications/%{_package_name}.desktop
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
