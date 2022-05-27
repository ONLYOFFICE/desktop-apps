#!/bin/sh
#

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
XDG_ICON_RESOURCE="$(which xdg-icon-resource 2> /dev/null || true)"
if [ ! -x "$XDG_ICON_RESOURCE" ]; then
  echo "Error: Could not find xdg-icon-resource" >&2
  exit 1
fi
for icon in "/opt/M4_DESKTOPEDITORS_PREFIX/asc-de-"*.png; do
  size="${icon##*/asc-de-}"
  "$XDG_ICON_RESOURCE" uninstall --size "${size%.png}" "M4_PACKAGE_NAME"
done

UPDATE_MENUS="$(which update-menus 2> /dev/null || true)"
if [ -x "$UPDATE_MENUS" ]; then
  update-menus
fi

# Update cache of .desktop file MIME types. Non-fatal since it's just a cache.
#update-desktop-database > /dev/null 2>&1 || true

