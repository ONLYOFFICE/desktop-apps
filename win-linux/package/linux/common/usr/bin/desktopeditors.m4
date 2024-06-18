#!/bin/sh

ifelse(M4_COMPANY_NAME, ONLYOFFICE,
set_names() {
  case $LANG in
    cs*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/cs-CZ"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Nový dokument"
      NEW_XLSX_NAME="Nový sešit"
      NEW_PPTX_NAME="Nová prezentace"
      NEW_DOCXF_NAME="Nový PDF formulář"
      ;;
    de*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/de-DE"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Neues Dokument"
      NEW_XLSX_NAME="Neues Tabellendokument"
      NEW_PPTX_NAME="Neue Präsentation"
      NEW_DOCXF_NAME="Neues PDF-Formular"
      ;;
    es*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/es-ES"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Documento nuevo"
      NEW_XLSX_NAME="Hoja de cálculo nueva"
      NEW_PPTX_NAME="Presentación nueva"
      NEW_DOCXF_NAME="Nuevo formulario PDF"
      ;;
    fr*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/fr-FR"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Nouveau document"
      NEW_XLSX_NAME="Nouveau classeur"
      NEW_PPTX_NAME="Nouvelle présentation"
      NEW_DOCXF_NAME="Nouveau formulaire PDF"
      ;;
    it*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/it-IT"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Nuovo documento"
      NEW_XLSX_NAME="Nuovo foglio di calcolo"
      NEW_PPTX_NAME="Nuova presentazione"
      NEW_DOCXF_NAME="Nuovo modulo PDF"
      ;;
    pt*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/pt-BR"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Novo Documento"
      NEW_XLSX_NAME="Nova planilha"
      NEW_PPTX_NAME="Nova apresentação"
      NEW_DOCXF_NAME="Novo formulário PDF"
      ;;
    ru*`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/ru-RU"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="Новый документ"
      NEW_XLSX_NAME="Новая эл.таблица"
      NEW_PPTX_NAME="Новая презентация"
      NEW_DOCXF_NAME="Новая PDF-форма"
      ;;
    *`)'
      SOURCE_DOC_DIR="$SOURCE_DOC_DIR/en-US"
      SOURCE_DOC_NAME="new"
      NEW_DOCX_NAME="New Document"
      NEW_XLSX_NAME="New Spreadsheet"
      NEW_PPTX_NAME="New Presentation"
      NEW_DOCXF_NAME="New PDF form"
      ;;
  esac
},
set_names_ru() {
  SOURCE_DOC_NAME="new"
  NEW_DOCX_NAME="Новый документ"
  NEW_XLSX_NAME="Новая эл.таблица"
  NEW_PPTX_NAME="Новая презентация"
})

check_templates() {
  if [ "$1" != "--new-document-templates" ]; then
    return 0
  fi

  SOURCE_DOC_DIR="/opt/M4_DESKTOPEDITORS_PREFIX/converter/empty"

  ifelse(M4_COMPANY_NAME, ONLYOFFICE,
  set_names,
  set_names_ru)

  eval TEMPLATE_DIR=$(grep XDG_TEMPLATES_DIR $HOME/.config/user-dirs.dirs | cut -d \" -f2)
  if [ $TEMPLATE_DIR = $HOME ]; then
    echo "system template's folder isn't found"
    return 0
  fi
  
  TEMPLATE_DOCX="$TEMPLATE_DIR/$NEW_DOCX_NAME.docx"
  TEMPLATE_XLSX="$TEMPLATE_DIR/$NEW_XLSX_NAME.xlsx"
  TEMPLATE_PPTX="$TEMPLATE_DIR/$NEW_PPTX_NAME.pptx"
  TEMPLATE_DOCXF="$TEMPLATE_DIR/$NEW_DOCXF_NAME.docxf"

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

  ifelse(M4_COMPANY_NAME, ONLYOFFICE,
  if [ $(ls -A $TEMPLATE_DIR/*.docxf 2>/dev/null | wc -l) -eq 0 ]
  then
    cp $SOURCE_DOC_DIR/$SOURCE_DOC_NAME.docxf "$TEMPLATE_DOCXF"
  fi)
}

check_templates "$@"

DIR=/opt/M4_DESKTOPEDITORS_PREFIX
if [ ! -z "$LD_LIBRARY_PATH" ]; then
  LDLPATH=:$LD_LIBRARY_PATH
fi
ifelse(M4_COMPANY_NAME, ONLYOFFICE,
export LD_LIBRARY_PATH=$DIR$LDLPATH,
DIR_MV=/opt/M4_MEDIAVIEWER_PREFIX
export LD_LIBRARY_PATH=$DIR:$DIR/converter:$DIR_MV$LDLPATH
export VLC_PLUGIN_PATH=$DIR_MV/plugins)
exec $DIR/DesktopEditors "$@"
