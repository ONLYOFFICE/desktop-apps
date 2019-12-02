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

override_dh_shlibdeps:
	dh_shlibdeps --no-act
