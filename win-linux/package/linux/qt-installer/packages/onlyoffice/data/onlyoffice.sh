#!/bin/bash
#
# Copyright (c) Ascensio System SIA, 2016
#
#

# This script is used as main entry point for the onlyoffice launching
# We override libc and ld library path, so need to `cd` to the install directory

# INSTALL_PATH is overriden during the installation phase
cd __INSTALL_PATH__

if [ -z "$1" ]; then
    ./"DesktopEditors.sh"
else
    ./"DesktopEditors.sh" "$1"
fi
