#!/bin/sh

DIR=/opt/M4_MEDIAVIEWER_PREFIX
if [ ! -z "$LD_LIBRARY_PATH" ]; then
  LDLPATH=:$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH=$DIR$LDLPATH
exec $DIR/ImageViewer "$@"
