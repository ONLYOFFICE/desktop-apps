param (
    [string]$Version = "0.0.0.0",
    [decimal]$Timestamp,
    [string]$UpdatesUrlPrefix,
    [string]$ReleaseNotesUrlPrefix,
    [switch]$Multilang,
    [string]$OutFile = "appcast.json"
)

$ErrorActionPreference = "Stop"
[Threading.Thread]::CurrentThread.CurrentCulture = 'en-US'
# [Threading.Thread]::CurrentThread.CurrentUICulture = 'en-US'
Set-Location $PSScriptRoot

if ( $Timestamp -eq 0 ) {
  $Timestamp = [Math]::Floor([decimal](Get-Date (Get-Date).ToUniversalTime() -UFormat "%s"))
}
$Date = (New-Object -Type DateTime -ArgumentList 1970, 1, 1, 0, 0, 0, 0).AddSeconds($Timestamp)
$PubDate = Get-Date $Date -UFormat "%b %d %H:%M UTC %Y"

$Content = @"
{
    "version": "${Version}",
    "date": "${PubDate}",
    "releaseNotes": {
        "en-EN": "${ReleaseNotesUrlPrefix}/changes.html",
"@
if ( $Multilang ) {
$Content += "`n" + @"
        "ru-RU": "${ReleaseNotesUrlPrefix}/changes_ru.html"
"@
}
$Content += "`n" + @"
    },
    "package": {
        "win_64": {
            "url": "${UpdatesUrlPrefix}/editors_update_x64.exe",
            "installArguments": "/silent /update"
        },
        "win_32": {
            "url": "${UpdatesUrlPrefix}/editors_update_x86.exe",
            "installArguments": "/silent /update"
        }
    }
}
"@

$Content
Set-Content -Path $OutFile -Value $Content
