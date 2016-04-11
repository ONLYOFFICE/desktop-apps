@echo off

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if %OS%==32BIT (
    set iss_compiler="C:\Program Files\Inno Setup 5\ISCC.exe"
    set proj_name=package/windows/install_x86.iss
    set pkg_name_ono=DesktopEditors_x86.exe
    set pkg_name_ivo=Ivolgapro_x86.exe
    set pkg_name_avs=AVSDocumentEditor_x86.exe
) else (
    set iss_compiler="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
    set proj_name=package/windows/install_x64.iss
    set pkg_name_ono=DesktopEditors_x64.exe
    set pkg_name_ivo=Ivolgapro_x64.exe
    set pkg_name_avs=AVSDocumentEditor_x64.exe
)

set /p APP_TYPE=what app type create (ivolga, avs, onlyoffice):
if %APP_TYPE%==ivolga (
    set CERT_NAME="NCT"
    set ISS_DEF=/d_IVOLGA_PRO
    set pkg_name=%pkg_name_ivo%
) else if %APP_TYPE%==avs (
    set CERT_NAME="OMT"
    set ISS_DEF=/d_AVS
    set pkg_name=%pkg_name_avs%
) else (
    set CERT_NAME="ASC"
    set ISS_DEF=
    set pkg_name=%pkg_name_ono%
)
@echo %pkg_name%

set devserver=192.168.3.118
set SignTool=\\%devserver%\Tools\DigitalSign\DigitalSignClient.exe
set Exchange=\\%devserver%\Exchange\Files\Office\Desktop

@echo.
@echo sign application and libraries
@echo.
pushd .\package\windows
call sign.bat %CERT_NAME%
popd

@echo compile project installation
%iss_compiler% %ISS_DEF% %proj_name%

if %errorlevel%==0 (
    copy /Y .\package\windows\%pkg_name% %Exchange%
    %SignTool% /input %Exchange%\%pkg_name% /company %CERT_NAME%

    if not %errorlevel%==0 (
        @echo installation signing failed: error %errorlevel%
        goto :exit
    )

    copy /Y %Exchange%\%pkg_name% .\package\windows\%pkg_name%

    rem if %errorlevel%==0 copy install\%pkg_name% \\MEDIASERVER\Exchange\Makc

    if %errorlevel%==0 (
        @echo no error
    ) else (
        @echo coping error: errorlevel
    )
) 

:exit
for %%f in (
    %Exchange%\%pkg_name%
) do (
    if exist %%f del %%f
)

@echo.
pause
