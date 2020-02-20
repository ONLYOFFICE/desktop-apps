#!/bin/sh

set_names() {
  case $LANG in
    cs*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/cs-CZ"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Nový dokument"
      NEW_XLSX_NAME="Nový sešit"
      NEW_PPTX_NAME="Nová prezentace"
      ;;
    de*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/de-DE"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Neues Dokument"
      NEW_XLSX_NAME="Neues Tabellendokument"
      NEW_PPTX_NAME="Neue Präsentation"
      ;;
    es*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/es-ES"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Documento nuevo"
      NEW_XLSX_NAME="Hoja de cálculo nueva"
      NEW_PPTX_NAME="Presentación nueva"
      ;;
    fr*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/fr-FR"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Nouveau document"
      NEW_XLSX_NAME="Nouveau classeur"
      NEW_PPTX_NAME="Nouvelle présentation"
      ;;
    it*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/it-IT"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Nuovo documento"
      NEW_XLSX_NAME="Nuovo foglio di calcolo"
      NEW_PPTX_NAME="Nuova presentazione"
      ;;
    pt*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/pt-BR"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Novo Documento"
      NEW_XLSX_NAME="Nova planilha"
      NEW_PPTX_NAME="Nova apresentação"
      ;;
    ru*)
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/ru-RU"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Новый документ"
      NEW_XLSX_NAME="Новая эл.таблица"
      NEW_PPTX_NAME="Новая презентация"
      ;;
    *)
      SOURCE_DOC_NAME="mm_new"
      NEW_DOCX_NAME="New Document"
      NEW_XLSX_NAME="New Spreadsheet"
      NEW_PPTX_NAME="New Presentation"
      ;;
  esac
}

set_names_ru() {
  SOURCE_DOC_NAME="new"
  NEW_DOCX_NAME="Новый документ"
  NEW_XLSX_NAME="Новая эл.таблица"
  NEW_PPTX_NAME="Новая презентация"
}

check_templates() {
  SOURCE_DOC_DIR="/opt/M4_DESKTOPEDITORS_PREFIX/converter/empty"

  ifelse(M4_COMPANY_NAME, ONLYOFFICE,
  set_names,
  set_names_ru)

  eval TEMPLATE_DIR=$(grep XDG_TEMPLATES_DIR $HOME/.config/user-dirs.dirs | cut -d \" -f2)
  TEMPLATE_DOCX="$TEMPLATE_DIR/$NEW_DOCX_NAME.docx"
  TEMPLATE_XLSX="$TEMPLATE_DIR/$NEW_XLSX_NAME.xlsx"
  TEMPLATE_PPTX="$TEMPLATE_DIR/$NEW_PPTX_NAME.pptx"

  mkdir -p $TEMPLATE_DIR

  if [ $(ls -A $TEMPLATE_DIR/*.docx 2>/dev/null | wc -l) -eq 0 ]
  then
    cp $SOURCE_DOC_DIR/$SOURCE_DOC_NAME.docx "$TEMPLATE_DOCX"
  fi

  if [ $(ls -A $TEMPLATE_DIR/*.xlsx 2>/dev/null | wc -l) -eq 0 ]
  then
    cp $SOURCE_DOC_DIR/$SOURCE_DOC_NAME.xlsx "$TEMPLATE_XLSX"
  fi

  if [ $(ls -A $TEMPLATE_DIR/*.pptx 2>/dev/null | wc -l) -eq 0 ]
  then
    cp $SOURCE_DOC_DIR/$SOURCE_DOC_NAME.pptx "$TEMPLATE_PPTX"
  fi
}

check_templates

DIR=/opt/M4_DESKTOPEDITORS_PREFIX
ifelse(M4_COMPANY_NAME, ONLYOFFICE,
export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH,
DIR_MV=/opt/M4_MEDIAVIEWER_PREFIX
export LD_LIBRARY_PATH=$DIR:$DIR/converter:$DIR_MV:$LD_LIBRARY_PATH
export VLC_PLUGIN_PATH=$DIR_MV/plugins)
exec $DIR/DesktopEditors "$@"
