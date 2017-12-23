PWD := $(shell pwd)
CURL := curl -L -o

PRODUCT_VERSION ?= 0.0.0
BUILD_NUMBER ?= 0

ifeq ($(OS),Windows_NT)
	PLATFORM := win
	EXEC_EXT := .exe
	SHELL_EXT := .bat
	SHARED_EXT := .dll
	LIB_EXT := .lib
	MAKE := nmake
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		PLATFORM := linux
		SHARED_EXT := .so*
		SHARED_SUFFIX := lib
		SHELL_EXT := .sh
		LIB_EXT := .a
		MAKE := make -j $(shell grep -c ^processor /proc/cpuinfo)
	endif
endif

UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
	ARCHITECTURE := 64
endif
ifneq ($(filter %86,$(UNAME_M)),)
	ARCHITECTURE := 32
endif

TARGET := $(PLATFORM)_$(ARCHITECTURE)

BINDIR := ../core/build/bin/$(TARGET)

ASCDOCUMENTEDITOR := $(BINDIR)/docbuilder$(EXEC_EXT)

TARGETS += $(ASCDOCUMENTEDITOR)

ASCDOCUMENTEDITOR_PRO := $(abspath win-linux/ASCDocumentEditor.pro)

QT_PROJ += ASCDOCUMENTEDITOR

#Template for next statment:
#FOO_MAKE := $(basename $(FOO_PRO)).build/Makefile
#$(FOO): $(FOO_MAKE)
#  cd $(dir $(FOO_MAKE)) && make

define build_proj_tmpl
PROS += $$(basename $$(value $(1)_PRO)).build
$(1)_MAKE := $$(basename $$(value $(1)_PRO)).build/Makefile
$$(value $(1)): $$(value $(1)_MAKE)
	cd $$(dir $$(value $(1)_MAKE)) && $(MAKE);
endef

.PHONY : all bin clean

all: bin

bin: $(ASCDOCUMENTEDITOR)

$(foreach proj, $(QT_PROJ), $(eval $(call build_proj_tmpl, $(proj))))


%.build/Makefile: %.pro
	mkdir -p $(dir $@) && cd $(dir $@) && PRODUCT_VERSION=$(PRODUCT_VERSION) BUILD_NUMBER=$(BUILD_NUMBER) qmake -r $<

clean:
	rm -rf $(TARGETS)
	for i in $(PROS); do \
		if [ -d $$i -a -f $$i/Makefile ]; then \
			cd $$i && $(MAKE) distclean; \
		fi \
done

