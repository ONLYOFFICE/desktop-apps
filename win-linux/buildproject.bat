@echo off

set compiler="C:\Qt\Qt5.5.1\Tools\QtCreator\bin\jom.exe"
set release_path=build\Release
set root_path=%~dp0
set parent_path="\\VBOXSVR\AscDocumentEditor\install"
set shared_path="\\MEDIASERVER\Exchange\Makc"

set /p is_update_libs=do you want to update libraries? (y, no):
set /p is_compile_app=do you want to compile the application? (y, no):
set /p brand=what brand do you prefer? (ivolga, avs, onlyoffice):

rem publish action is: 1 - copy from virtual box, 
rem 2 - publish particular installation, 3 - publish all installation
rem set /p is_build_install=(3-3) build installation package? (0: no, 1: from vbox, 2: current install, 3: all installs):
set is_build_install=0

if %is_update_libs%==0 if %is_compile_app%==0 if %is_build_install%==0 goto :exit

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if %OS%==32BIT (
    set qmake="C:\Qt\Qt5.5.1\5.5\msvc2013\bin\qmake.exe" 
    set vcvars="C:\Program Files\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
    set iscc="C:\Program Files\Inno Setup 5\ISCC.exe"
    set iss_project=install_x86.iss
    set inst_name=DesktopEditors_x86.exe
) else (
    set qmake="C:\Qt\Qt5.5.1\5.5\msvc2013_64\bin\qmake.exe" 
    set vcvars="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
    set iscc="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
    set iss_project=install_x64.iss
    set inst_name=DesktopEditors_x64.exe
)

:: update libraries
if %is_update_libs%==y (
    pushd ..\common\libs 
    call update.bat
    popd

    if not errorlevel  0 (
        @echo update libraries failed: %errorlevel%
        goto :exit
    )
)

:: compile application
if %brand%==ivolga (
    set def="DEFINES+=_IVOLGA_PRO" "DEFINES-=_AVS"
) else if %brand%==avs (
    set def="DEFINES-=_IVOLGA_PRO" "DEFINES+=_AVS" "CONFIG+=avs_theme"
) else (
    set def="DEFINES-=_IVOLGA_PRO _AVS"
)

if %is_compile_app%==y (
    md %release_path%
    pushd %release_path%
    %qmake% ../../ASCDocumentEditor.pro  -r -spec win32-msvc2013 %def%

    if not errorlevel==0 (
        @echo qmake failed: %errorlevel%
        goto :exit
    )

    rem apply compiler environment variables
    call %vcvars%

    %compiler% -F Makefile.Release clean
    %compiler% -F Makefile.Release
    if not errorlevel  0 (
        @echo compiler failed: %errorlevel%
        goto :exit
    )

    popd
)

:: build installation package
if 1==0 (
rem    if not exist "loginpage\deploy\index.html" (
rem        @echo deploy the login page

rem        cd loginpage\build
rem        call grunt
rem        cd ../..
rem    )

    cd install
    %iscc% %iss_project%

    if not errorlevel 0 (
        @echo creating installation package failed with error: %errorlevel%
        goto :exit
    ) 

    :: copy installation from virtual box.
    :: in that case coping in network path is too long
    if %is_build_install%==1 (
        copy %inst_name% %parent_path%
    )

    :: copy installation to shared directory.
    if %is_build_install%==2 (
        copy %inst_name% %shared_path%
    )

    :: publish installations to a shared directory
    if %is_build_install%==3 (
        copy DesktopEditors*.exe %shared_path%
    )
)

:exit
cd %root_path%
pause
::goto :eof
