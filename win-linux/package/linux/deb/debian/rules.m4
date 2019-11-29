#!/usr/bin/make -f
# -*- makefile -*-

# Uncomment this to turn on verbose mode.
#export DH_VERBOSE=1

%:
	dh $@

override_dh_shlibdeps:
	dh_shlibdeps --dpkg-shlibdeps-params="--ignore-missing-info" -l"$(shell pwd)/debian/M4_PACKAGE_NAME/opt/M4_DESKTOPEDITORS_PREFIX:$(shell pwd)/debian/M4_PACKAGE_NAME/opt/M4_DESKTOPEDITORS_PREFIX/swiftshader:$(shell pwd)/debian/M4_PACKAGE_NAME/opt/M4_DESKTOPEDITORS_PREFIX/platforminputcontexts:$(shell pwd)/debian/M4_PACKAGE_NAME/opt/M4_DESKTOPEDITORS_PREFIX/converter"
