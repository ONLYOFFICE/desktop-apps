@echo off

setlocal enabledelayedexpansion

set devserver=192.168.3.118
set Exchange=\\%devserver%\Exchange\Files\Office\Desktop
set APP_EXE=..\build\Release\release\DesktopEditors.exe
set SIGN_TOOL=\\%devserver%\Tools\DigitalSign\DigitalSignClient.exe

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if %OS%==32BIT (
    set PATH_DLLs=..\..\common\converter\windows\win32
    set PATH_CORELIBs=..\..\common\libs\ChromiumBasedEditors2\app\corebuilds\win32\
) else (
    set PATH_DLLs=..\..\common\converter\windows\win64
    set PATH_CORELIBs=..\..\common\libs\ChromiumBasedEditors2\app\corebuilds\win64\
)

:: OMT, ASC, NCT
set CERT_NAME="%1"

call:funcSingleSing %APP_EXE%
call:funcSingleSing %PATH_CORELIBs%\ascdocumentscore.dll
del /q %Exchange%\*.*

pushd %PATH_DLLs%
xcopy /y *.* %Exchange% /exclude:..\exclude.txt
popd
for /f %%f in ('dir /b %Exchange%') do (
	@echo.
	@echo sining file: %%~nxf

    %SIGN_TOOL% /input %Exchange%\%%f /company %CERT_NAME%
    if not !errorlevel! equ 0 (
        @echo saining failed: %%f
        goto exit /b 1
    )
)

xcopy /y %Exchange%\*.*  %PATH_DLLs%
del /q %Exchange%\*.*

rem pause&goto:eof
goto:eof

:funcSingleSing
@echo.
@echo sining file: %~nx1

copy %~1 %Exchange%\%~nx1
%SIGN_TOOL% /input %Exchange%\%~nx1 /company %CERT_NAME%

if not !errorlevel! equ 0 (
    @echo.
    @echo sining failed: %~nx1
    goto exit /b 1
)
copy %Exchange%\%~nx1 %~1
goto :eof