param (
    [Parameter(Mandatory)][string]$Target,
    [string]$BuildDir = "build",
    [string]$DesktopDir = "DesktopEditors",
    [string]$MultimediaDir,
    [switch]$ExcludeHelp,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$Suffix = switch ( $Target )
{
    "windows_x64"    { "x64" }
    "windows_x86"    { "x86" }
    "windows_x64_xp" { "x64-xp" }
    "windows_x86_xp" { "x86-xp" }
}

# Check directory
if ( -Not (Test-Path -Path $BuildDir) ) {
    Write-Error "Path $BuildDir does not exist"
}

if ( $Sign ) {
    if ( $DesktopDir ) {
        Set-Location "$BuildDir\$DesktopDir"

        $SignFiles = Get-ChildItem *.exe, *.dll, converter\*.exe, converter\*.dll `
            | Resolve-Path -Relative
        # Sign
        Write-Host "signtool sign /a /n $CertName /t $TimestampServer $SignFiles" -ForegroundColor Yellow
        & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # Verify
        Write-Host "signtool verify /pa /all $SignFiles" -ForegroundColor Yellow
        & signtool verify /pa /all $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }

        Set-Location $PSScriptRoot
    }

    if ( $MultimediaDir ) {
        Set-Location "$BuildDir\$MultimediaDir"

        $SignFiles = Get-ChildItem *.exe, *.dll | Resolve-Path -Relative
        # Sign
        Write-Host "signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles" -ForegroundColor Yellow
        & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # Verify
        Write-Host "signtool verify /pa /all $SignFiles" -ForegroundColor Yellow
        & signtool verify /pa /all $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }

        Set-Location $PSScriptRoot
    }
}

# Create archive

$OutFile = "$Env:COMPANY_NAME-$DesktopDir-$Env:PRODUCT_VERSION.$Env:BUILD_NUMBER-$Suffix.zip"
if ( !$ExcludeHelp ) {
    Write-Host "7z a -y $OutFile .\$BuildDir\$DesktopDir\*" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\$DesktopDir\*
} else {
    Write-Host "7z a -y $OutFile .\$BuildDir\$DesktopDir\* -xr!editors\web-apps\apps\*\main\resources\help" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\$DesktopDir\* -xr!editors\web-apps\apps\*\main\resources\help
}
if ( $LastExitCode -ne 0 ) { throw }

if ( $MultimediaDir ) {
    $OutFile = "$Env:COMPANY_NAME-$MultimediaDir-$Env:PRODUCT_VERSION.$Env:BUILD_NUMBER-$Suffix.zip"
    Write-Host "7z a -y $OutFile .\$BuildDir\$MultimediaDir\*" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\$MultimediaDir\*
    if ( $LastExitCode -ne 0 ) { throw }
}
