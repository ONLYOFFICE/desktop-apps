param (
    [string]$Title = "Desktop Editors",
    [string]$Version = "0.0.0.0",
    [decimal]$Timestamp,
    [string]$UpdatesUrlPrefix,
    [string]$ReleaseNotesUrlPrefix,
    [switch]$Multilang,
    [string]$OutFile = "appcast.xml"
)

$ErrorActionPreference = "Stop"
[Threading.Thread]::CurrentThread.CurrentCulture = 'en-US'
# [Threading.Thread]::CurrentThread.CurrentUICulture = 'en-US'
Set-Location $PSScriptRoot

if ( $Timestamp -eq 0 ) {
    $Timestamp = [Math]::Floor([decimal](Get-Date (Get-Date).ToUniversalTime() -UFormat "%s"))
}
$Date = (New-Object -Type DateTime -ArgumentList 1970, 1, 1, 0, 0, 0, 0).AddSeconds($Timestamp)
$PubDate = Get-Date $Date -UFormat "%a, %d %b %Y %H:%M:%S +0000"

$Content = @"
<?xml version="1.0" encoding="UTF-8"?>
<rss
    version="2.0"
    xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle"
    xmlns:dc="http://purl.org/dc/elements/1.1/">
  <channel>
    <title>${Title} Changelog</title>
    <description>Most recent changes with links to updates.</description>
    <language>en</language>
    <item>
      <title>Version ${Version}</title>
      <pubDate>${PubDate}</pubDate>
      <sparkle:releaseNotesLink>
        ${ReleaseNotesUrlPrefix}/changes.html
      </sparkle:releaseNotesLink>
"@
if ( $Multilang ) {
$Content += "`n" + @"
      <sparkle:releaseNotesLink xml:lang="ru-RU">
        ${ReleaseNotesUrlPrefix}/changes_ru.html
      </sparkle:releaseNotesLink>
"@
}
$Content += "`n" + @"
      <enclosure url="${UpdatesUrlPrefix}/editors_update_x64.exe"
                 sparkle:os="windows-x64"
                 sparkle:version="${Version}"
                 sparkle:shortVersionString="${Version}"
                 sparkle:installerArguments="/silent /update"
                 length="0"
                 type="application/octet-stream" />
    </item>
    <item>
      <title>Version ${Version}</title>
      <pubDate>${PubDate}</pubDate>
      <sparkle:releaseNotesLink>
        ${ReleaseNotesUrlPrefix}/changes.html
      </sparkle:releaseNotesLink>
"@
if ( $Multilang ) {
$Content += "`n" + @"
      <sparkle:releaseNotesLink xml:lang="ru-RU">
        ${ReleaseNotesUrlPrefix}/changes_ru.html
      </sparkle:releaseNotesLink>
"@
}
$Content += "`n" + @"
      <enclosure url="${UpdatesUrlPrefix}/editors_update_x86.exe"
                 sparkle:os="windows-x86"
                 sparkle:version="${Version}"
                 sparkle:shortVersionString="${Version}"
                 sparkle:installerArguments="/silent /update"
                 length="0"
                 type="application/octet-stream" />
    </item>
  </channel>
</rss>
"@

$Content
Set-Content -Path $OutFile -Value $Content
