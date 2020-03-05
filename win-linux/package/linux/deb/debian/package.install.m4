../common/opt/desktopeditors/* /opt/M4_DESKTOPEDITORS_PREFIX/
../common/usr/bin/M4_DESKTOPEDITORS_EXEC /usr/bin/
../common/usr/share/applications/M4_DESKTOPEDITORS_EXEC.desktop /usr/share/applications/

ifelse(M4_COMPANY_NAME, ONLYOFFICE,
../common/usr/bin/desktopeditors /usr/bin/,
../common/opt/mediaviewer/* /opt/M4_MEDIAVIEWER_PREFIX/
../common/usr/bin/M4_IMAGEVIEWER_EXEC /usr/bin/
../common/usr/bin/M4_VIDEOPLAYER_EXEC /usr/bin/
../common/usr/share/applications/M4_IMAGEVIEWER_EXEC.desktop /usr/share/applications/
../common/usr/share/applications/M4_VIDEOPLAYER_EXEC.desktop /usr/share/applications/
../common/usr/share/flyfm/templates/desktopeditors.*.desktop /usr/share/flyfm/templates/
../common/usr/bin/M4_PACKAGE_NAME /usr/bin/)
