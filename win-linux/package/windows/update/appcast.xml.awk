BEGIN {
	Title = AppcastProductTitle " Changelog";
	PubDate = strftime("%a, %e %b %Y %H:%M:%S %z", Timestamp, "0");
	SparkleVersion = Version "." Build;

	print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	print "<rss version=\"2.0\"" \
		" xmlns:sparkle=\"http://www.andymatuschak.org/xml-namespaces/sparkle\"" \
		" xmlns:dc=\"http://purl.org/dc/elements/1.1/\">";
	print "    <channel>";
	print "        <title>" Title "</title>";
	print "        <description>Most recent changes with links to updates." \
		"</description>";
	print "        <language>en</language>";
	print "        <item>";

	print "            <title>Version " Version "</title>";
	print "            <sparkle:releaseNotesLink>" \
		AppcastChangesPath "/changes.html</sparkle:releaseNotesLink>";
	if (onlyoffice)
		print "            <sparkle:releaseNotesLink xml:lang=\"ru-RU\">" \
			AppcastChangesPath "/changes_ru.html</sparkle:releaseNotesLink>";
	print "            <pubDate>" PubDate "</pubDate>";
	print "            <enclosure" \
		" url=\"" AppcastUpdatesPath "/editors_update_x64.exe\"" \
		" sparkle:os=\"windows-x64\"" \
		" sparkle:version=\"" SparkleVersion "\"" \
		" sparkle:shortVersionString=\"" SparkleVersion "\"" \
		" sparkle:installerArguments=\"/silent /update\"" \
		" length=\"0\"" \
		" type=\"application/octet-stream\" />";

	print "        </item>";
	print "        <item>";

	print "            <title>Version " Version "</title>";
	print "            <sparkle:releaseNotesLink>" \
		AppcastChangesPath "/changes.html</sparkle:releaseNotesLink>";
	if (onlyoffice)
		print "            <sparkle:releaseNotesLink xml:lang=\"ru-RU\">" \
			AppcastChangesPath "/changes_ru.html</sparkle:releaseNotesLink>";
	print "            <pubDate>" PubDate "</pubDate>";
	print "            <enclosure" \
		" url=\"" AppcastUpdatesPath "/editors_update_x86.exe\"" \
		" sparkle:os=\"windows-x86\"" \
		" sparkle:version=\"" SparkleVersion "\"" \
		" sparkle:shortVersionString=\"" SparkleVersion "\"" \
		" sparkle:installerArguments=\"/silent /update\"" \
		" length=\"0\"" \
		" type=\"application/octet-stream\" />";

	print "        </item>";
	print "    </channel>";
	print "</rss>";
}
