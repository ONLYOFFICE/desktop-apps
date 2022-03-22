#!/usr/bin/make -f
# -*- makefile -*-

export DH_VERBOSE=1

%:
	dh $@

override_dh_fixperms:
	dh_fixperms
	chmod 755 debian/M4_PACKAGE_NAME/opt/M4_DESKTOPEDITORS_PREFIX/DesktopEditors
	ifelse(M4_COMPANY_NAME, ONLYOFFICE,,
	chmod 755 debian/M4_PACKAGE_NAME/opt/M4_MEDIAVIEWER_PREFIX/ImageViewer
	chmod 755 debian/M4_PACKAGE_NAME/opt/M4_MEDIAVIEWER_PREFIX/VideoPlayer
	chmod 777 debian/M4_PACKAGE_NAME/etc/M4_PACKAGE_NAME)

ifdef(`M4_ASTRALINUX_SIGN_IMAGE',
override_dh_strip:
	dh_strip
	docker run --rm \
		-v M4_ASTRALINUX_KEYS_DIR:/root/keys \
		-v $(shell pwd)/debian/M4_PACKAGE_NAME:/root/buildroot \
		--name sign-app \
		M4_ASTRALINUX_SIGN_IMAGE,)

override_dh_shlibdeps:
