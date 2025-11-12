include(defines.m4)dnl
[Desktop Entry]
Version=1.0
Name=DESKTOPEDITORS_NAME
GenericName=defn('DESKTOPEDITORS[GenericName[en]]')
ifdef('DESKTOPEDITORS[GenericName[de]]',GenericName[de]=defn('DESKTOPEDITORS[GenericName[de]]'),'dnl')
ifdef('DESKTOPEDITORS[GenericName[fr]]',GenericName[fr]=defn('DESKTOPEDITORS[GenericName[fr]]'),'dnl')
ifdef('DESKTOPEDITORS[GenericName[es]]',GenericName[es]=defn('DESKTOPEDITORS[GenericName[es]]'),'dnl')
ifdef('DESKTOPEDITORS[GenericName[ru]]',GenericName[ru]=defn('DESKTOPEDITORS[GenericName[ru]]'),'dnl')
Comment=defn('DESKTOPEDITORS[Comment[en]]')
ifdef('DESKTOPEDITORS[Comment[de]]',Comment[de]=defn('DESKTOPEDITORS[Comment[de]]'),'dnl')
ifdef('DESKTOPEDITORS[Comment[fr]]',Comment[fr]=defn('DESKTOPEDITORS[Comment[fr]]'),'dnl')
ifdef('DESKTOPEDITORS[Comment[es]]',Comment[es]=defn('DESKTOPEDITORS[Comment[es]]'),'dnl')
ifdef('DESKTOPEDITORS[Comment[ru]]',Comment[ru]=defn('DESKTOPEDITORS[Comment[ru]]'),'dnl')
Type=Application
Exec=/usr/bin/M4_DESKTOPEDITORS_EXEC %U
Terminal=false
Icon=M4_DESKTOPEDITORS_EXEC
Keywords=Text;Document;OpenDocument Text;Microsoft Word;Microsoft Works;odt;doc;docx;rtf;
Categories=Office;WordProcessor;Spreadsheet;Presentation;
MimeType=dnl
application/msword;dnl
application/msword-template;dnl
application/vnd.ms-word.document.macroEnabled.12;dnl
application/vnd.ms-word.template.macroEnabled.12;dnl
application/vnd.ms-xpsdocument;dnl
application/vnd.ms-excel;dnl
application/vnd.ms-excel.sheet.macroEnabled.12;dnl
application/vnd.ms-excel.sheet.binary.macroEnabled.12;dnl
application/vnd.ms-excel.template.macroEnabled.12;dnl
application/vnd.ms-powerpoint;dnl
application/vnd.ms-powerpoint.presentation.macroEnabled.12;dnl
application/vnd.ms-powerpoint.slideshow.macroEnabled.12;dnl
application/vnd.ms-powerpoint.template.macroEnabled.12;dnl
application/vnd.ms-visio.drawing.main+xml;dnl
application/vnd.ms-visio.drawing.macroEnabled.main+xml;dnl
application/vnd.ms-visio.stencil.main+xml;dnl
application/vnd.ms-visio.stencil.macroEnabled.main+xml;dnl
application/vnd.ms-visio.template.main+xml;dnl
application/vnd.ms-visio.template.macroEnabled.main+xml;dnl
application/vnd.openxmlformats-officedocument.wordprocessingml.document;dnl
application/vnd.openxmlformats-officedocument.wordprocessingml.template;dnl
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet;dnl
application/vnd.openxmlformats-officedocument.spreadsheetml.template;dnl
application/vnd.openxmlformats-officedocument.presentationml.presentation;dnl
application/vnd.openxmlformats-officedocument.presentationml.slideshow;dnl
application/vnd.openxmlformats-officedocument.presentationml.template;dnl
application/vnd.apple.pages;dnl
application/vnd.apple.numbers;dnl
application/vnd.apple.keynote;dnl
application/vnd.oasis.opendocument.text;dnl
application/vnd.oasis.opendocument.text-flat-xml;dnl
application/vnd.oasis.opendocument.text-template;dnl
application/vnd.oasis.opendocument.spreadsheet;dnl
application/vnd.oasis.opendocument.spreadsheet-flat-xml;dnl
application/vnd.oasis.opendocument.spreadsheet-template;dnl
application/vnd.oasis.opendocument.presentation;dnl
application/vnd.oasis.opendocument.presentation-flat-xml;dnl
application/vnd.oasis.opendocument.presentation-template;dnl
application/vnd.oasis.opendocument.graphics;dnl
application/vnd.sun.xml.writer;dnl
application/vnd.sun.xml.writer.template;dnl
application/vnd.sun.xml.calc;dnl
application/vnd.sun.xml.impress;dnl
application/wps-office.wps;dnl
application/wps-office.wpt;dnl
application/wps-office.et;dnl
application/wps-office.ett;dnl
application/wps-office.dps;dnl
application/wps-office.dpt;dnl
application/x-hwp;dnl
application/epub+zip;dnl
application/oxps;dnl
application/pdf;dnl
application/rtf;dnl
application/x-fictionbook+xml;dnl
image/vnd.djvu;dnl
text/csv;dnl
text/markdown;dnl
text/plain;dnl
text/tab-separated-values;dnl
x-scheme-handler/M4_SCHEME_HANDLER;
Actions=NewDocument;NewSpreadsheet;NewPresentation;ifelse(M4_COMPANY_NAME, ONLYOFFICE, NewForm;)
StartupWMClass=DESKTOPEDITORS_WM_CLASS

[Desktop Action NewDocument]
Name=defn('NEWDOCUMENT[Name[en]]')
ifdef('NEWDOCUMENT[Name[de]]',Name[de]=defn('NEWDOCUMENT[Name[de]]'),'dnl')
ifdef('NEWDOCUMENT[Name[fr]]',Name[fr]=defn('NEWDOCUMENT[Name[fr]]'),'dnl')
ifdef('NEWDOCUMENT[Name[es]]',Name[es]=defn('NEWDOCUMENT[Name[es]]'),'dnl')
ifdef('NEWDOCUMENT[Name[ru]]',Name[ru]=defn('NEWDOCUMENT[Name[ru]]'),'dnl')
Exec=/usr/bin/M4_DESKTOPEDITORS_EXEC --new:word

[Desktop Action NewSpreadsheet]
Name=defn('NEWSPREADSHEET[Name[en]]')
ifdef('NEWSPREADSHEET[Name[de]]',Name[de]=defn('NEWSPREADSHEET[Name[de]]'),'dnl')
ifdef('NEWSPREADSHEET[Name[fr]]',Name[fr]=defn('NEWSPREADSHEET[Name[fr]]'),'dnl')
ifdef('NEWSPREADSHEET[Name[es]]',Name[es]=defn('NEWSPREADSHEET[Name[es]]'),'dnl')
ifdef('NEWSPREADSHEET[Name[ru]]',Name[ru]=defn('NEWSPREADSHEET[Name[ru]]'),'dnl')
Exec=/usr/bin/M4_DESKTOPEDITORS_EXEC --new:cell

[Desktop Action NewPresentation]
Name=defn('NEWPRESENTATION[Name[en]]')
ifdef('NEWPRESENTATION[Name[de]]',Name[de]=defn('NEWPRESENTATION[Name[de]]'),'dnl')
ifdef('NEWPRESENTATION[Name[fr]]',Name[fr]=defn('NEWPRESENTATION[Name[fr]]'),'dnl')
ifdef('NEWPRESENTATION[Name[es]]',Name[es]=defn('NEWPRESENTATION[Name[es]]'),'dnl')
ifdef('NEWPRESENTATION[Name[ru]]',Name[ru]=defn('NEWPRESENTATION[Name[ru]]'),'dnl')
Exec=/usr/bin/M4_DESKTOPEDITORS_EXEC --new:slide

ifelse(M4_COMPANY_NAME, ONLYOFFICE,
[Desktop Action NewForm]
Name=defn('NEWFORM[Name[en]]')
ifdef('NEWFORM[Name[de]]',Name[de]=defn('NEWFORM[Name[de]]'),'dnl')
ifdef('NEWFORM[Name[fr]]',Name[fr]=defn('NEWFORM[Name[fr]]'),'dnl')
ifdef('NEWFORM[Name[es]]',Name[es]=defn('NEWFORM[Name[es]]'),'dnl')
ifdef('NEWFORM[Name[ru]]',Name[ru]=defn('NEWFORM[Name[ru]]'),'dnl')
Exec=/usr/bin/M4_DESKTOPEDITORS_EXEC --new:form)
