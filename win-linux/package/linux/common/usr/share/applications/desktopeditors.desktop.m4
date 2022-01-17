include(variables.m4)dnl
[Desktop Entry]
Version=1.0
ifelse(M4_COMPANY_NAME, ONLYOFFICE,
Name=M4_COMPANY_NAME M4_PRODUCT_NAME,
Name=defn('DESKTOPEDITORS[Name[en]]')
ifdef('DESKTOPEDITORS[Name[de]]',Name[de]=defn('DESKTOPEDITORS[Name[de]]'),'dnl')
ifdef('DESKTOPEDITORS[Name[fr]]',Name[fr]=defn('DESKTOPEDITORS[Name[fr]]'),'dnl')
ifdef('DESKTOPEDITORS[Name[es]]',Name[es]=defn('DESKTOPEDITORS[Name[es]]'),'dnl')
ifdef('DESKTOPEDITORS[Name[ru]]',Name[ru]=defn('DESKTOPEDITORS[Name[ru]]'),'dnl'))
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
Icon=M4_PACKAGE_NAME
Keywords=Text;Document;OpenDocument Text;Microsoft Word;Microsoft Works;odt;doc;docx;rtf;
Categories=Office;WordProcessor;Spreadsheet;Presentation;
MimeType=application/vnd.oasis.opendocument.text;application/vnd.oasis.opendocument.text-template;application/vnd.oasis.opendocument.text-web;application/vnd.oasis.opendocument.text-master;application/vnd.sun.xml.writer;application/vnd.sun.xml.writer.template;application/vnd.sun.xml.writer.global;application/msword;application/vnd.ms-word;application/x-doc;application/rtf;text/rtf;application/vnd.wordperfect;application/wordperfect;application/vnd.openxmlformats-officedocument.wordprocessingml.document;application/vnd.ms-word.document.macroenabled.12;application/vnd.openxmlformats-officedocument.wordprocessingml.template;application/vnd.ms-word.template.macroenabled.12;application/vnd.oasis.opendocument.spreadsheet;application/vnd.oasis.opendocument.spreadsheet-template;application/vnd.sun.xml.calc;application/vnd.sun.xml.calc.template;application/msexcel;application/vnd.ms-excel;application/vnd.openxmlformats-officedocument.spreadsheetml.sheet;application/vnd.ms-excel.sheet.macroenabled.12;application/vnd.openxmlformats-officedocument.spreadsheetml.template;application/vnd.ms-excel.template.macroenabled.12;application/vnd.ms-excel.sheet.binary.macroenabled.12;text/csv;text/spreadsheet;application/csv;application/excel;application/x-excel;application/x-msexcel;application/x-ms-excel;text/comma-separated-values;text/tab-separated-values;text/x-comma-separated-values;text/x-csv;application/vnd.oasis.opendocument.presentation;application/vnd.oasis.opendocument.presentation-template;application/vnd.sun.xml.impress;application/vnd.sun.xml.impress.template;application/mspowerpoint;application/vnd.ms-powerpoint;application/vnd.openxmlformats-officedocument.presentationml.presentation;application/vnd.ms-powerpoint.presentation.macroenabled.12;application/vnd.openxmlformats-officedocument.presentationml.template;application/vnd.ms-powerpoint.template.macroenabled.12;application/vnd.openxmlformats-officedocument.presentationml.slide;application/vnd.openxmlformats-officedocument.presentationml.slideshow;application/vnd.ms-powerpoint.slideshow.macroEnabled.12;x-scheme-handler/oo-office;text/docxf;text/oform;
Actions=NewDocument;NewSpreadsheet;NewPresentation;

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
