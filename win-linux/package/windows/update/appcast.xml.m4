changequote()changequote(`,`)dnl
define(`APPCAST_TITLE`,format(`%s Changelog`,M4_COMPANY_NAME M4_PRODUCT_NAME))dnl
define(`APPCAST_DESCRIPTION`,`Most recent changes with links to updates.`)dnl
define(`APPCAST_VERSION`,M4_PACKAGE_VERSION)dnl
define(`APPCAST_ITEM_TITLE`,format(`Version %s`,patsubst(M4_PACKAGE_VERSION,`\(\.\w+\)$`)))dnl
define(`APPCAST_PUBDATE`,esyscmd(echo -n $(LANG=en_US date -u -d @M4_BUILD_TIMESTAMP "+%b %e %Y")))dnl
define(`APPCAST_LINK_CHANGES_EN`,`http://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/changes/changes.html`)dnl
define(`APPCAST_LINK_CHANGES_RU`,`http://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/changes/changes_ru.html`)dnl
define(`APPCAST_LINK_EXE_64`,`http://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/updates/editors_update_x64.exe`)dnl
define(`APPCAST_LINK_EXE_32`,`http://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/updates/editors_update_x86.exe`)dnl
<?xml version="1.0" encoding="UTF-8"?>
<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle" xmlns:dc="http://purl.org/dc/elements/1.1/">
    <channel>
        <title>APPCAST_TITLE</title>
        <description>APPCAST_DESCRIPTION</description>
        <language>en</language>
        <item>
            <title>APPCAST_ITEM_TITLE</title>
            <pubDate>APPCAST_PUBDATE</pubDate>
            <sparkle:releaseNotesLink>APPCAST_LINK_CHANGES_EN</sparkle:releaseNotesLink>
            <sparkle:releaseNotesLink xml:lang="ru-RU">APPCAST_LINK_CHANGES_RU</sparkle:releaseNotesLink>
            <enclosure sparkle:os="windows-x64" sparkle:version="APPCAST_VERSION" sparkle:shortVersionString="APPCAST_VERSION" url="APPCAST_LINK_EXE_64" length="0" type="application/octet-stream" sparkle:installerArguments="/silent /update"/>
        </item>
        <item>
            <title>APPCAST_ITEM_TITLE</title>
            <pubDate>APPCAST_PUBDATE</pubDate>
            <sparkle:releaseNotesLink>APPCAST_LINK_CHANGES_EN</sparkle:releaseNotesLink>
            <sparkle:releaseNotesLink xml:lang="ru-RU">APPCAST_LINK_CHANGES_RU</sparkle:releaseNotesLink>
            <enclosure sparkle:os="windows-x86" sparkle:version="APPCAST_VERSION" sparkle:shortVersionString="APPCAST_VERSION" url="APPCAST_LINK_EXE_32" length="0" type="application/octet-stream" sparkle:installerArguments="/silent /update"/>
        </item>
    </channel>
</rss>
