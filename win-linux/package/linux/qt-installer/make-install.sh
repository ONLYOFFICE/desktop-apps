#!/bin/bash
#
# Copyright (c) Ascensio System SIA, 2016
#
#

BASE_DIR="$(cd "$(dirname -- "$0")" && pwd)"
BUILD_DIR=build-installer
LINUX_DESKTOP_BINARIES_DIR=../../../../../core/build/linux_desktop
LINUX_DESKTOP_FULL_DIR=../../../../../core/build/linux_desktop/desktop_full
PACKAGE_DATA_DIR=$BUILD_DIR/packages/onlyoffice/data

if [ ! -d $LINUX_DESKTOP_BINARIES_DIR ]; then
	echo "ERROR: Linux desktop binaries is not exist"
	exit 1
fi

# Prepare temerary directory
echo "Prepare temerary directory"
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
mkdir -p $BUILD_DIR/packages/onlyoffice/data

# Build and copy linux application
#echo "Build and copy linux application"
#(cd $LINUX_DESKTOP_BINARIES_DIR && ./deploy.sh)

# Copy application to the installer directory
echo "Copy application to the installer directory"
cp -vR $LINUX_DESKTOP_FULL_DIR/* $PACKAGE_DATA_DIR
chmod -R +x $PACKAGE_DATA_DIR

# Copy installer files to the installer directory
echo "Copy installer files to the installer directory"
cp -vR config $BUILD_DIR
cp -vR packages $BUILD_DIR

# Create installation
echo "Create installation"
(cd $BUILD_DIR && binarycreator -v -f -c config/config.xml -p packages ../onlyoffice-installer-x64)

# Cleanup
echo "Cleanup"
rm -rf $BUILD_DIR

echo "Done"
