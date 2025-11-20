#!/bin/sh

copy_templates() {
  ifelse(M4_COMPANY_NAME, ONLYOFFICE,
  case $LANG in
    ar_* `)'
      TEMPLATE_LANG="ar-SA"
      NEW_DOCX="مستند جديد"
      NEW_XLSX="جدول بياني جديد"
      NEW_PPTX="عرض تقديمي جديد"
      NEW_PDFF="PDF جديد"
      ;;
    be_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="Новы дакумент"
      NEW_XLSX="Новая электронная табліца"
      NEW_PPTX="Новая прэзентацыя"
      NEW_PDFF="Новый PDF"
      ;;
    bg_* `)'
      TEMPLATE_LANG="bg-BG"
      NEW_DOCX="Нов документ"
      NEW_XLSX="Нова електронна таблица"
      NEW_PPTX="Нова презентация"
      NEW_PDFF="Нов PDF"
      ;;
    ca_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="Nou document"
      NEW_XLSX="Nou full de càlcul"
      NEW_PPTX="Nova presentació"
      NEW_PDFF="Nou PDF"
      ;;
    cs_* `)'
      TEMPLATE_LANG="cs-CZ"
      NEW_DOCX="Nový dokument"
      NEW_XLSX="Nový sešit"
      NEW_PPTX="Nová prezentace"
      NEW_PDFF="Nový PDF"
      ;;
    da_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="Nyt dokument"
      NEW_XLSX="Nyt regneark"
      NEW_PPTX="Ny præsentation"
      NEW_PDFF="Nyt PDF"
      ;;
    de_* `)'
      TEMPLATE_LANG="de-DE"
      NEW_DOCX="Neues Dokument"
      NEW_XLSX="Neue Tabelle"
      NEW_PPTX="Neue Präsentation"
      NEW_PDFF="Neues PDF"
      ;;
    el_* `)'
      TEMPLATE_LANG="el-GR"
      NEW_DOCX="Νέο έγγραφο"
      NEW_XLSX="Νέο λογιστικό φύλλο"
      NEW_PPTX="Νέα παρουσίαση"
      NEW_PDFF="Νέο PDF"
      ;;
    en_GB* `)'
      TEMPLATE_LANG="en-GB"
      NEW_DOCX="New document"
      NEW_XLSX="New spreadsheet"
      NEW_PPTX="New presentation"
      NEW_PDFF="New PDF"
      ;;
    en_* `)'
      TEMPLATE_LANG="en-US"
      NEW_DOCX="New document"
      NEW_XLSX="New spreadsheet"
      NEW_PPTX="New presentation"
      NEW_PDFF="New PDF"
      ;;
    es_* `)'
      TEMPLATE_LANG="es-ES"
      NEW_DOCX="Nuevo Documento"
      NEW_XLSX="Nueva Hoja de Cálculo"
      NEW_PPTX="Nueva Presentación"
      NEW_PDFF="Nuevo PDF"
      ;;
    fi_* `)'
      TEMPLATE_LANG="fi-FI"
      NEW_DOCX="Uusi asiakirja"
      NEW_XLSX="Uusi laskentataulukko"
      NEW_PPTX="Uusi esitys"
      NEW_PDFF="Uusi PDF"
      ;;
    fr_* `)'
      TEMPLATE_LANG="fr-FR"
      NEW_DOCX="Nouveau document"
      NEW_XLSX="Nouvelle feuille de calcul"
      NEW_PPTX="Nouvelle présentation"
      NEW_PDFF="Nouveau PDF"
      ;;
    gl_* `)'
      TEMPLATE_LANG="gl-ES"
      NEW_DOCX="Novo documento"
      NEW_XLSX="Nova folla de cálculo"
      NEW_PPTX="Nova presentación"
      NEW_PDFF="Novo PDF"
      ;;
    he_* `)'
      TEMPLATE_LANG="he-IL"
      NEW_DOCX="מסמך חדש"
      NEW_XLSX="גיליון אלקטרוני חדש"
      NEW_PPTX="מצגת חדשה"
      NEW_PDFF="PDF חדש"
      ;;
    hu_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="Új dokumentum"
      NEW_XLSX="Új táblázat"
      NEW_PPTX="Új bemutató"
      NEW_PDFF="Új PDF"
      ;;
    hy_* `)'
      TEMPLATE_LANG="hy-AM"
      NEW_DOCX="Նոր փաստաթուղթ"
      NEW_XLSX="Նոր աղյուսակաթերթ"
      NEW_PPTX="Նոր ներկայացում"
      NEW_PDFF="Նոր PDF"
      ;;
    id_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="Dokumen baru"
      NEW_XLSX="Lembar kerja baru"
      NEW_PPTX="Presentasi Baru"
      NEW_PDFF="PDF baru"
      ;;
    it_* `)'
      TEMPLATE_LANG="it-IT"
      NEW_DOCX="Nuovo documento"
      NEW_XLSX="Nuovo foglio elettronico"
      NEW_PPTX="Nuova presentazione"
      NEW_PDFF="Nuovo PDF"
      ;;
    ja_* `)'
      TEMPLATE_LANG="ja-JP"
      NEW_DOCX="新しいドキュメント"
      NEW_XLSX="新しいスプレッドシート"
      NEW_PPTX="新しいプレゼンテーション"
      NEW_PDFF="新しいPDF"
      ;;
    ko_* `)'
      TEMPLATE_LANG="ko-KR"
      NEW_DOCX="신규 문서"
      NEW_XLSX="새로운 스프레드 시트"
      NEW_PPTX="새 프리젠 테이션"
      NEW_PDFF="새로운 PDF"
      ;;
    lo_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="ເອກະສານໃໝ່"
      NEW_XLSX="ຕາຕະລາງໃໝ່"
      NEW_PPTX="ບົດນຳສະເໜີໃໝ່"
      NEW_PDFF="PDF ໃໝ່"
      ;;
    lv_* `)'
      TEMPLATE_LANG="lv-LV"
      NEW_DOCX="Jauns dokuments"
      NEW_XLSX="Jauna tabula"
      NEW_PPTX="Jauna prezentācija"
      NEW_PDFF="Jauns PDF"
      ;;
    nb_* `)'
      TEMPLATE_LANG="nb-NO"
      NEW_DOCX="Nytt dokument"
      NEW_XLSX="Nieuw werkblad"
      NEW_PPTX="Nieuwe presentatie"
      NEW_PDFF="Nieuw PDF"
      ;;
    nl_* `)'
      TEMPLATE_LANG="nl-NL"
      NEW_DOCX="Nieuw Document"
      NEW_XLSX="Nieuw werkblad"
      NEW_PPTX="Nieuwe presentatie"
      NEW_PDFF="Nieuw PDF"
      ;;
    pl_* `)'
      TEMPLATE_LANG="pl-PL"
      NEW_DOCX="Nowy dokument"
      NEW_XLSX="Nowy arkusz kalkulacyjny"
      NEW_PPTX="Nowa prezentacja"
      NEW_PDFF="Nowy PDF"
      ;;
    pt_BR* `)'
      TEMPLATE_LANG="pt-BR"
      NEW_DOCX="Novo Documento"
      NEW_XLSX="Nova planilha"
      NEW_PPTX="Nova apresentação"
      NEW_PDFF="Novo PDF"
      ;;
    pt_* `)'
      TEMPLATE_LANG="pt-PT"
      NEW_DOCX="Novo documento"
      NEW_XLSX="Nova folha de cálculo"
      NEW_PPTX="Nova Apresentação"
      NEW_PDFF="Novo PDF"
      ;;
    ro_* `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="Document nou"
      NEW_XLSX="Foaie de calcul nouă"
      NEW_PPTX="Prezentare nouă"
      NEW_PDFF="PDF nou"
      ;;
    ru_* `)'
      TEMPLATE_LANG="ru-RU"
      NEW_DOCX="Новый документ"
      NEW_XLSX="Новая таблица"
      NEW_PPTX="Новая презентация"
      NEW_PDFF="Новый PDF"
      ;;
    si_* `)'
      TEMPLATE_LANG="si-LK"
      NEW_DOCX="නව ලේඛනය"
      NEW_XLSX="නව පැතුරුම්පත"
      NEW_PPTX="නව සමර්පණය"
      NEW_PDFF="නව PDF"
      ;;
    sk_* `)'
      TEMPLATE_LANG="sk-SK"
      NEW_DOCX="Nový dokument"
      NEW_XLSX="Nová tabuľka"
      NEW_PPTX="Nová prezentácia"
      NEW_PDFF="Nový PDF"
      ;;
    sl_* `)'
      TEMPLATE_LANG="sl-SI"
      NEW_DOCX="Nov dokument"
      NEW_XLSX="Nova razpredelnica"
      NEW_PPTX="Nova predstavitev"
      NEW_PDFF="Novi PDF"
      ;;
    sr_RS@latin* `)'
      TEMPLATE_LANG="sr-Latn-RS"
      NEW_DOCX="Novi dokument"
      NEW_XLSX="Nova proračunska tabela"
      NEW_PPTX="Nova prezentacija"
      NEW_PDFF="Novi PDF"
      ;;
    sr_RS* `)'
      TEMPLATE_LANG="sr-Cyrl-RS"
      NEW_DOCX="Нови документ"
      NEW_XLSX="Нова прорачунска табела"
      NEW_PPTX="Нова презентација"
      NEW_PDFF="Нови PDF"
      ;;
    sv_* `)'
      TEMPLATE_LANG="sv-SE"
      NEW_DOCX="Nytt dokument"
      NEW_XLSX="Nytt kalkylblad"
      NEW_PPTX="Ny presentation"
      NEW_PDFF="Nytt PDF"
      ;;
    tr_* `)'
      TEMPLATE_LANG="tr-TR"
      NEW_DOCX="Yeni Belge"
      NEW_XLSX="Yeni Hesap Tablosu"
      NEW_PPTX="Yeni Sunum"
      NEW_PDFF="Yeni PDF"
      ;;
    uk_* `)'
      TEMPLATE_LANG="uk-UA"
      NEW_DOCX="Новий документ"
      NEW_XLSX="Нова таблиця"
      NEW_PPTX="Нова презентація"
      NEW_PDFF="Новий PDF"
      ;;
    vi_* `)'
      TEMPLATE_LANG="vi-VN"
      NEW_DOCX="Tài liệu mới"
      NEW_XLSX="Bảng tính mới"
      NEW_PPTX="Bản trình chiếu mới"
      NEW_PDFF="PDF mới"
      ;;
    zh_CN* `)'
      TEMPLATE_LANG="zh-CN"
      NEW_DOCX="新建文档"
      NEW_XLSX="新建表格"
      NEW_PPTX="新建幻灯片"
      NEW_PDFF="新建 PDF"
      ;;
    zh_TW* `)'
      TEMPLATE_LANG="zh-TW"
      NEW_DOCX="新文件"
      NEW_XLSX="新試算表"
      NEW_PPTX="新簡報"
      NEW_PDFF="新的 PDF"
      ;;
    * `)'
      TEMPLATE_LANG="default"
      NEW_DOCX="New document"
      NEW_XLSX="New spreadsheet"
      NEW_PPTX="New presentation"
      NEW_PDFF="New PDF"
      ;;
  esac
  SOURCE_DIR="/opt/M4_DESKTOPEDITORS_PREFIX/converter/empty/$TEMPLATE_LANG",
  TEMPLATE_LANG="ru-RU"
  NEW_DOCX="Новый документ"
  NEW_XLSX="Новая таблица"
  NEW_PPTX="Новая презентация"
  SOURCE_DIR="/opt/M4_DESKTOPEDITORS_PREFIX/converter/empty")

  if command -v xdg-user-dir &> /dev/null; then
    XDG_TEMPLATES_DIR=$(xdg-user-dir TEMPLATES)
  else
    echo "xdg-user-dir not installed"
    exit 1
  fi

  mkdir -pv "$XDG_TEMPLATES_DIR"
  cp -fv "$SOURCE_DIR/new.docx" "$XDG_TEMPLATES_DIR/$NEW_DOCX.docx"
  cp -fv "$SOURCE_DIR/new.xlsx" "$XDG_TEMPLATES_DIR/$NEW_XLSX.xlsx"
  cp -fv "$SOURCE_DIR/new.pptx" "$XDG_TEMPLATES_DIR/$NEW_PPTX.pptx"
  ifelse(M4_COMPANY_NAME, ONLYOFFICE,
  cp -fv "$SOURCE_DIR/new.pdf" "$XDG_TEMPLATES_DIR/$NEW_PDFF.pdf")

  exit 0
}

for arg in "$@"; do
  if [ "$arg" = "--new-document-templates" ]; then
    copy_templates
  fi
done

DIR=/opt/M4_DESKTOPEDITORS_PREFIX
ifelse(M4_COMPANY_NAME, ONLYOFFICE,
export LD_LIBRARY_PATH=$DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH},
DIR_MV=/opt/M4_MEDIAVIEWER_PREFIX
export LD_LIBRARY_PATH=$DIR:$DIR/converter:$DIR_MV$LDLPATH
export VLC_PLUGIN_PATH=$DIR_MV/plugins)
exec $DIR/DesktopEditors "$@"
