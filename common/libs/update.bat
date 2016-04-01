@echo off
set fileserver=192.168.3.15

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT

set trunk_path=svn://%fileserver%/onlyoffice/build
set export_path=..\..\..\core\Build
if %OS%==32BIT (
    set OSNAME=win_32
) else (
    set OSNAME=win_64
)

md %export_path%
pushd %export_path%
if exist ".svn" (
    svn update --accept tf
) else (
    svn checkout %trunk_path% ./. --depth empty

    svn update --set-depth empty bin
    svn update --set-depth infinity bin/windows

    svn update --set-depth empty bin/icu
    svn update --set-depth infinity bin/icu/%OSNAME%

    svn update --set-depth empty cef
    svn update --set-depth infinity cef/%OSNAME%

    svn update --set-depth infinity empty
    svn update --set-depth infinity jsbuilds

    svn update --set-depth empty lib
    svn update --set-depth infinity lib/%OSNAME%
)
popd
