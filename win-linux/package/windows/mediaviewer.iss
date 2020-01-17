
[CustomMessages]
;======================================================================================================
en.daImageViewerDescription=Viewer for photos and pictures of popular formats
;cs_CZ.daImageViewerDescription=Viewer for photos and pictures of popular formats
;sk.daImageViewerDescription=Viewer for photos and pictures of popular formats
ru.daImageViewerDescription=Приложение для просмотра фотографий и изображений популярных форматов
;de.daImageViewerDescription=Viewer for photos and pictures of popular formats
;fr.daImageViewerDescription=Viewer for photos and pictures of popular formats
;es.daImageViewerDescription=Viewer for photos and pictures of popular formats
;it_IT.daImageViewerDescription=Viewer for photos and pictures of popular formats
;pt_BR.daImageViewerDescription=Viewer for photos and pictures of popular formats
;======================================================================================================
en.daVideoPlayerDescription=Player for multimedia files
;cs_CZ.daVideoPlayerDescription=Player for multimedia files
;sk.daVideoPlayerDescription=Player for multimedia files
ru.daVideoPlayerDescription=Приложение для прослушивания музыки и просмотра видеороликов
;de.daVideoPlayerDescription=Player for multimedia files
;fr.daVideoPlayerDescription=Player for multimedia files
;es.daVideoPlayerDescription=Player for multimedia files
;it_IT.daVideoPlayerDescription=Player for multimedia files
;pt_BR.daVideoPlayerDescription=Player for multimedia files

[Code]
function getMediaViewerPath(param: string): string;
begin
  result := ExpandFileName(ExpandConstant('{app}\{#sAppSubDir}'));
end;

[Files]
Source: data\vcredist\vcredist_2013_{#sWinArch}.exe; DestDir: {app}; Flags: deleteafterinstall; \
  AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_2013_{#sWinArch}.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); \
  Check: not checkVCRedist2013;

#ifndef SCRIPT_CUSTOM_FILES
Source: ..\..\deploy\{#sPlatform}\videoplayer.dll;                          DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\desktop-sdk-wrapper\plugins\*;                     DestDir: {app}\editors\sdkjs-plugins; Flags: ignoreversion recursesubdirs;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\mediaservice\*;     DestDir: {app}\mediaservice; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\plugins\*;          DestDir: {app}\plugins; Flags: ignoreversion recursesubdirs;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\libvlc.dll;         DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\libvlccore.dll;     DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\VLCQtCore.dll;      DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\VLCQtWidgets.dll;   DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\Qt5Multimedia.dll;  DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\Qt5Network.dll;     DestDir: {app}; Flags: ignoreversion;
Source: ..\..\..\..\core\multimedia\deploy\{#sPlatform}\Qt5MultimediaWidgets.dll; DestDir: {app}; Flags: ignoreversion;
#endif

[Registry]
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: mediapath;  ValueData: {code:getMediaViewerPath}; Flags: uninsdeletevalue;
