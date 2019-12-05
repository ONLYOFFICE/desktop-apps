#!/bin/sh

DIR=/opt/M4_DESKTOPEDITORS_PREFIX
export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH
exec $DIR/DesktopEditors "$@"
