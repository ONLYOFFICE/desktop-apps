#!/usr/bin/make -f
# -*- makefile -*-

export DH_VERBOSE=1

%:
	dh $@

override_dh_shlibdeps:
	dh_shlibdeps --no-act
