#!/bin/sh

TEMPLATE_DIR="$HOME/Templates"
SOURCE_DOC_DIR="/opt/onlyoffice/desktopeditors/converter/empty"
TEMPLATE_DOCX="$TEMPLATE_DIR/New-Document.docx"
TEMPLATE_XLSX="$TEMPLATE_DIR/New-Spreadsheet.xlsx"
TEMPLATE_PPTX="$TEMPLATE_DIR/New-Presentation.pptx"

if ! [ -f $TEMPLATE_DOCX ]
then
  cp $SOURCE_DOC_DIR/mm_new.docx $TEMPLATE_DOCX
fi

if ! [ -f $TEMPLATE_XLSX ]
then
  cp $SOURCE_DOC_DIR/mm_new.xlsx $TEMPLATE_XLSX
fi

if ! [ -f $TEMPLATE_PPTX ]
then
  cp $SOURCE_DOC_DIR/mm_new.pptx $TEMPLATE_PPTX
fi

DIR=/opt/M4_DESKTOPEDITORS_PREFIX
ifelse(M4_COMPANY_NAME, ONLYOFFICE,
export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH,
DIR_MV=/opt/M4_MEDIAVIEWER_PREFIX
export LD_LIBRARY_PATH=$DIR:$DIR/converter:$DIR_MV:$LD_LIBRARY_PATH
export VLC_PLUGIN_PATH=$DIR_MV/plugins)
exec $DIR/DesktopEditors "$@"
