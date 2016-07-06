#!/bin/bash
#
# Copyright (c) Ascensio System SIA, 2016
#

# This script is used to set up environment for actual app launching
set -euf

APPNAME="DesktopEditors"
DIRNAME="$(cd "$(dirname -- "$0")" && pwd)"

(LD_LIBRARY_PATH="$DIRNAME" exec "$DIRNAME/$APPNAME" "$@")
