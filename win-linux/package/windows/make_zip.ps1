param (
    [string]$DesktopPath = "build\DesktopEditors",
    [Parameter(Mandatory)][string]$OutFile,
    [switch]$ExcludeHelp,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

# Check directory
if ( -Not (Test-Path -Path $DesktopPath) ) {
    Write-Error "Path $DesktopPath does not exist"
}

if ( $Sign ) {
    Set-Location $DesktopPath

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

# Create archive
if ( !$ExcludeHelp ) {
    Write-Host "7z a -y $OutFile .\$DesktopPath\*" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$DesktopPath\*
} else {
    Write-Host "7z a -y $OutFile .\$DesktopPath\* -xr!editors\web-apps\apps\*\main\resources\help" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$DesktopPath\* -xr!editors\web-apps\apps\*\main\resources\help
}
if ( $LastExitCode -ne 0 ) { throw }
