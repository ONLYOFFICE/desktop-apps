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

MIMEAPPS_LIST="/usr/share/applications/mimeapps.list"
if [ ! -f "$MIMEAPPS_LIST" ]; then
  echo "[Default Applications]" >"$MIMEAPPS_LIST"
fi
if [ $(cat "$MIMEAPPS_LIST" | grep x-scheme-handler/oo-office | wc -l) -eq "0" ]; then
  echo "x-scheme-handler/oo-office=onlyoffice-desktopeditors.desktop" >>"$MIMEAPPS_LIST"
fi
if [ $(cat "$MIMEAPPS_LIST" | grep text/docxf | wc -l) -eq "0" ]; then
  echo "text/docxf=onlyoffice-desktopeditors.desktop" >>"$MIMEAPPS_LIST"
fi
if [ $(cat "$MIMEAPPS_LIST" | grep text/oform | wc -l) -eq "0" ]; then
  echo "text/oform=onlyoffice-desktopeditors.desktop" >>"$MIMEAPPS_LIST"
fi

xdg-mime install --mode system /opt/M4_DESKTOPEDITORS_PREFIX/mimetypes/onlyoffice-docxf.xml
xdg-mime install --mode system /opt/M4_DESKTOPEDITORS_PREFIX/mimetypes/onlyoffice-oform.xml

# Update cache of .desktop file MIME types. Non-fatal since it's just a cache.
#update-desktop-database > /dev/null 2>&1 || true
