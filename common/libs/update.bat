@echo off
set fileserver=192.168.3.15

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT

set trunk_path=svn://%fileserver%/onlyoffice/build
set export_path=.
if %OS%==32BIT (
    set OSNAME=win_32
) else (
    set OSNAME=win_64
)

md %export_path%\core\%OSNAME%\debug
md %export_path%\converter\%OSNAME%
md %export_path%\cef\%OSNAME%

if %OS%==32BIT (
    svn export --force %trunk_path%/bin/windows/x2t32.exe %export_path%\converter\%OSNAME%
) else (
    svn export --force %trunk_path%/bin/windows/x2t.exe %export_path%\converter\%OSNAME%
)

svn export --force %trunk_path%/lib/%OSNAME%/ %export_path%\converter\%OSNAME%
svn export --force %trunk_path%/bin/icu/%OSNAME%/ %export_path%\converter\%OSNAME%

pushd %export_path%\converter\%OSNAME%
move /y .\ascdocumentscore.* ..\..\core\%OSNAME%
move /y .\debug\ascdocumentscore.* ..\..\core\%OSNAME%\debug
rd /s /q .\debug
del .\*.lib .\ascdocumentscore.*
popd

svn export --force %trunk_path%/cef/%OSNAME%/ %export_path%/cef/%OSNAME%

svn export --force %trunk_path%/jsbuilds %export_path%/jsbuilds
svn export --force %trunk_path%/bin/windows/icudt.dll %export_path%/converter/%OSNAME%
svn export --force %trunk_path%/empty %export_path%/converter/%OSNAME%/empty
