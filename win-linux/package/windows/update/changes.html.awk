BEGIN {
	if (Lang == "") Lang = ENVIRON["LANG"];
	if (Lang == "en_US.UTF-8") {
		Title = ChangesProductTitle " Release Notes";
		ReleaseDate = strftime("%B %e, %Y", Timestamp, "0");
		More = "and a little more...";
	}
	else if (Lang == "ru_RU.UTF-8") {
		Title = "История изменений " ChangesProductTitle;
		ReleaseDate = strftime("%e %B %Y", Timestamp, "0");
		More = "и многое другое...";
	}

	print "<!DOCTYPE html>";
	print "<html>";
	print "<head>";
	print "    <meta charset=\"utf-8\">";
	print "    <title>" Title "</title>";
	print "    <style type=\"text/css\">";
	print "        body {";
	print "            background: white;";
	print "            font: 12px \"Lucida Grande\"," \
		" \"Lucida Sans Unicode\", Verdana, Lucida, Helvetica, sans-serif;";
	print "        }";
	print "        h1, h2, h3 {";
	print "            color: #000000;";
	print "            font-family: \"Helvetica\";";
	print "            font-weight: normal;";
	print "            font-style: normal;";
	print "        }";
	print "        h1 {";
	print "            font-size: 18px;";
	print "        }";
	print "        h2 {";
	print "            font-size: 16px;";
	print "        }";
	print "        h3 {";
	print "            font-size: 14px;";
	print "        }";
	print "        .releasedate {";
	print "            color: #888;";
	print "            font-size: medium;";
	print "        }";
	print "        .more {";
	print "            margin-bottom: 20px;";
	print "        }";
	print "        .version {";
	print "            border-bottom: 1px solid #cfcfcf;";
	print "        }";
	print "        code {";
	print "            background: var(--color-gray-200);";
	print "            font-family: monospace;";
	print "            padding: 1px 5px;";
	print "        }";
	print "    </style>";
	print "</head>";
	print "<body>";
	print "    <div class=\"version\">";
	print "        <h1>" ChangesProductHeading " " Version \
		"<span class=\"releasedate\"> - " ReleaseDate "</span></h1>";

	# if (system("test -r " ARGV[1]) != 0) {
	# 	print "        <p>empty</p>"
	# 	exit
	# }
}
BEGINFILE {
	if (ERRNO != "") {
		print "        <p>" ERRNO "</p>";
		nextfile;
	}
}
{
	print "        " $0;
}
END {
	if (onlyoffice)
		print "        <div class=\"more\"><a href=\"" ChangesMoreUrl "\"" \
			" target=\"_blank\">" More "</a></div>";
	print "    </div>";
	print "</body>";
	print "</html>";
}
