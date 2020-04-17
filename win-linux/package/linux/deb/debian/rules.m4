#!/usr/bin/make -f
# -*- makefile -*-

# Uncomment this to turn on verbose mode.
#export DH_VERBOSE=1

%:
	dh $@

override_dh_installdocs:
	dh_installdocs --no-act

override_dh_installchangelogs:
	dh_installchangelogs --no-act

override_dh_fixperms:
	dh_fixperms
	chmod 755 debian/M4_PACKAGE_NAME/opt/M4_DESKTOPEDITORS_PREFIX/DesktopEditors
	ifelse(M4_COMPANY_NAME, ONLYOFFICE,,
	chmod 755 debian/M4_PACKAGE_NAME/opt/M4_MEDIAVIEWER_PREFIX/ImageViewer
	chmod 755 debian/M4_PACKAGE_NAME/opt/M4_MEDIAVIEWER_PREFIX/VideoPlayer
	chmod 777 debian/M4_PACKAGE_NAME/etc/M4_PACKAGE_NAME)

override_dh_shlibdeps:
	dh_shlibdeps --no-act
