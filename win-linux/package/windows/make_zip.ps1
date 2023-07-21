param (
    [string]$BuildDir = "build",
    [string]$DesktopDir = "DesktopEditors",
    [string]$MultimediaDir,
    [Parameter(Mandatory)][string]$OutFile,
    [switch]$ExcludeHelp,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

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
        Write-Host "signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles" -ForegroundColor Yellow
        & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # Verify
        Write-Host "signtool verify /pa /all /v $SignFiles" -ForegroundColor Yellow
        & signtool verify /pa /all /v $SignFiles
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
        Write-Host "signtool verify /pa /all /v $SignFiles" -ForegroundColor Yellow
        & signtool verify /pa /all /v $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }

        Set-Location $PSScriptRoot
    }
}

# Create archive
if ( !$ExcludeHelp ) {
    Write-Host "7z a -y $OutFile .\$BuildDir\*" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\*
} else {
    Write-Host "7z a -y $OutFile .\$BuildDir\* -xr!$DesktopDir\editors\web-apps\apps\*\main\resources\help" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\* -xr!$DesktopDir\editors\web-apps\apps\*\main\resources\help
}
if ( $LastExitCode -ne 0 ) { throw }
