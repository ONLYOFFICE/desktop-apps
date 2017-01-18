#!/bin/bash
#
# Copyright (c) Ascensio System SIA, 2017
#
#

IFW_INSTALL_PATH=$1
IFW_CONFIG_DIR=$2

# This script is running as root (not from sudo), so we need to know actual user for some operations
IFW_CURRENT_USER=$(who | awk '{print $1}' | head -1)

# Linux distrubution name (f.e. "ubuntu", "astra", ...)
IFW_LINUX_DISTRIBUTION=$(cat /etc/*-release | grep "^ID=" | sed "s|ID=||g")

# Removing Desktop icons
su - "${IFW_CURRENT_USER}" -c "xdg-desktop-icon uninstall /usr/share/applications/onlyoffice.desktop"

# Unregister in desktop environment
xdg-desktop-menu uninstall --mode system "${IFW_INSTALL_PATH}/onlyoffice.desktop"

# Restoring font config changes
restoreFontConfig()
{
    local fontConfigFolder="/etc/fonts/conf.avail"

    if [ -f "${fontConfigFolder}/${1}.backup" ]; then
        mv "${fontConfigFolder}/${1}.backup" "${fontConfigFolder}/${1}"
    fi
}

if [[ "$IFW_LINUX_DISTRIBUTION" == "astra" ]]; then
    restoreFontConfig "19-fly-no-antialiasing.conf" # Used in version 1.4
    restoreFontConfig "29-fly-rendering.conf" # Used in version 1.5
fi

# Removing user config
rm -rf "/home/${IFW_CURRENT_USER}/.config/${IFW_CONFIG_DIR}"
