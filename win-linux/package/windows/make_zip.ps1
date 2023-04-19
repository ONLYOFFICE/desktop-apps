param (
    [string]$DesktopPath = "build\desktop",
    [Parameter(Mandatory)][string]$OutFile,
    [switch]$Sign = $false,
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

    $SignFiles = Get-ChildItem *.exe, converter\*.exe, converter\*.dll, `
        ascdocumentscore.dll, hunspell.dll, ooxmlsignature.dll |
        Resolve-Path -Relative
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
Write-Host "7z a -y $OutFile $DesktopPath\*" -ForegroundColor Yellow
& 7z a -y $OutFile $DesktopPath\*
if ( $LastExitCode -ne 0 ) { throw }
