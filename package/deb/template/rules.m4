#!/usr/bin/make -f
# -*- makefile -*-

export DH_VERBOSE=1

%:
	dh $@

override_dh_strip_nondeterminism:
	dh_strip_nondeterminism --exclude=.png

override_dh_shlibdeps:
	dh_shlibdeps --no-act

DPKG_THREADS ?= $(shell n=$$(nproc); [ $$n -lt 4 ] && echo $$n || echo 4)

override_dh_builddeb:
	dh_builddeb -- -z9 -Zxz --threads-max=$(DPKG_THREADS)
