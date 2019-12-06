#!/bin/sh

DIR=/opt/M4_DESKTOPEDITORS_PREFIX
ifelse(M4_COMPANY_NAME, ONLYOFFICE,
export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH,
DIR_MV=/opt/M4_MEDIAVIEWER_PREFIX
export LD_LIBRARY_PATH=$DIR:$DIR/converter:$DIR_MV/mediaviewer:$LD_LIBRARY_PATH
export VLC_PLUGIN_PATH=$DIR_MV/mediaviewer/plugins)
exec $DIR/DesktopEditors "$@"
