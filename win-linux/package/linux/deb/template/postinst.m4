#!/bin/sh
#

set -e 		# fail on any error
set -u 		# treat unset variable as errors

# Add icons to the system icons
XDG_ICON_RESOURCE="$(which xdg-icon-resource 2> /dev/null || true)"
if [ ! -x "$XDG_ICON_RESOURCE" ]; then
  echo "Error: Could not find xdg-icon-resource" >&2
  exit 1
fi
for icon in "/opt/M4_DESKTOPEDITORS_PREFIX/asc-de-"*.png; do
  size="${icon##*/asc-de-}"
  "$XDG_ICON_RESOURCE" install --size "${size%.png}" "$icon" "M4_PACKAGE_NAME"
done

UPDATE_MENUS="$(which update-menus 2> /dev/null || true)"
if [ -x "$UPDATE_MENUS" ]; then
  update-menus
fi

ifelse(M4_COMPANY_NAME, ONLYOFFICE,
xdg-mime install --mode system /opt/M4_DESKTOPEDITORS_PREFIX/mimetypes/onlyoffice-docxf.xml
xdg-mime install --mode system /opt/M4_DESKTOPEDITORS_PREFIX/mimetypes/onlyoffice-oform.xml
update-mime-database /usr/share/mime
update-desktop-database /usr/share/applications
xdg-mime default M4_DESKTOPEDITORS_EXEC.desktop application/pdf,
if [ -f /etc/astra_version ] && [ -f /etc/X11/trusted ]; then
  sed -i '\|/opt/M4_DESKTOPEDITORS_PREFIX/DesktopEditors|d' /etc/X11/trusted
  echo '/opt/M4_DESKTOPEDITORS_PREFIX/DesktopEditors(KBD_R)' >> /etc/X11/trusted
fi)

# Update cache of .desktop file MIME types. Non-fatal since it's just a cache.
#update-desktop-database > /dev/null 2>&1 || true
