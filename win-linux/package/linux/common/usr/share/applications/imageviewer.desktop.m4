include(variables.m4)dnl
[Desktop Entry]
Version=1.0
Name=defn('IMAGEVIEWER[Name[en]]')
ifdef('IMAGEVIEWER[Name[de]]',Name[de]=defn('IMAGEVIEWER[Name[de]]'),'dnl')
ifdef('IMAGEVIEWER[Name[fr]]',Name[fr]=defn('IMAGEVIEWER[Name[fr]]'),'dnl')
ifdef('IMAGEVIEWER[Name[es]]',Name[es]=defn('IMAGEVIEWER[Name[es]]'),'dnl')
ifdef('IMAGEVIEWER[Name[ru]]',Name[ru]=defn('IMAGEVIEWER[Name[ru]]'),'dnl')
GenericName=defn('IMAGEVIEWER[GenericName[en]]')
ifdef('IMAGEVIEWER[GenericName[de]]',GenericName[de]=defn('IMAGEVIEWER[GenericName[de]]'),'dnl')
ifdef('IMAGEVIEWER[GenericName[fr]]',GenericName[fr]=defn('IMAGEVIEWER[GenericName[fr]]'),'dnl')
ifdef('IMAGEVIEWER[GenericName[es]]',GenericName[es]=defn('IMAGEVIEWER[GenericName[es]]'),'dnl')
ifdef('IMAGEVIEWER[GenericName[ru]]',GenericName[ru]=defn('IMAGEVIEWER[GenericName[ru]]'),'dnl')
Comment=defn('IMAGEVIEWER[Comment[en]]')
ifdef('IMAGEVIEWER[Comment[de]]',Comment[de]=defn('IMAGEVIEWER[Comment[de]]'),'dnl')
ifdef('IMAGEVIEWER[Comment[fr]]',Comment[fr]=defn('IMAGEVIEWER[Comment[fr]]'),'dnl')
ifdef('IMAGEVIEWER[Comment[es]]',Comment[es]=defn('IMAGEVIEWER[Comment[es]]'),'dnl')
ifdef('IMAGEVIEWER[Comment[ru]]',Comment[ru]=defn('IMAGEVIEWER[Comment[ru]]'),'dnl')
Type=Application
Exec=/usr/bin/M4_IMAGEVIEWER_EXEC %F
Terminal=false
Icon=M4_PACKAGE_NAME
Keywords=Image;Photo;Picture;Viewer;
Categories=Graphics;
MimeType=image/jpg;image/jpeg;image/gif;image/png;image/bmp;image/tiff;
