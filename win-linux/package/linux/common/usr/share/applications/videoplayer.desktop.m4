include(variables.m4)dnl
[Desktop Entry]
Version=1.0
Name=defn('VIDEOPLAYER[Name[en]]')
ifdef('VIDEOPLAYER[Name[de]]',Name[de]=defn('VIDEOPLAYER[Name[de]]'),'dnl')
ifdef('VIDEOPLAYER[Name[fr]]',Name[fr]=defn('VIDEOPLAYER[Name[fr]]'),'dnl')
ifdef('VIDEOPLAYER[Name[es]]',Name[es]=defn('VIDEOPLAYER[Name[es]]'),'dnl')
ifdef('VIDEOPLAYER[Name[ru]]',Name[ru]=defn('VIDEOPLAYER[Name[ru]]'),'dnl')
GenericName=defn('VIDEOPLAYER[GenericName[en]]')
ifdef('VIDEOPLAYER[GenericName[de]]',GenericName[de]=defn('VIDEOPLAYER[GenericName[de]]'),'dnl')
ifdef('VIDEOPLAYER[GenericName[fr]]',GenericName[fr]=defn('VIDEOPLAYER[GenericName[fr]]'),'dnl')
ifdef('VIDEOPLAYER[GenericName[es]]',GenericName[es]=defn('VIDEOPLAYER[GenericName[es]]'),'dnl')
ifdef('VIDEOPLAYER[GenericName[ru]]',GenericName[ru]=defn('VIDEOPLAYER[GenericName[ru]]'),'dnl')
Comment=defn('VIDEOPLAYER[Comment[en]]')
ifdef('VIDEOPLAYER[Comment[de]]',Comment[de]=defn('VIDEOPLAYER[Comment[de]]'),'dnl')
ifdef('VIDEOPLAYER[Comment[fr]]',Comment[fr]=defn('VIDEOPLAYER[Comment[fr]]'),'dnl')
ifdef('VIDEOPLAYER[Comment[es]]',Comment[es]=defn('VIDEOPLAYER[Comment[es]]'),'dnl')
ifdef('VIDEOPLAYER[Comment[ru]]',Comment[ru]=defn('VIDEOPLAYER[Comment[ru]]'),'dnl')
Type=Application
Exec=/usr/bin/M4_VIDEOPLAYER_EXEC %F
Terminal=false
Icon=M4_PACKAGE_NAME
Keywords=Video;Audio;Sound;Movie;Player;
Categories=AudioVideo;Video;Audio;
MimeType=video/x-msvideo;video/mp4;video/mpeg;audio/mpeg;video/ogg;
