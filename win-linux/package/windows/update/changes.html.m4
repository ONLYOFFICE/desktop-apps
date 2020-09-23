changequote(',')dnl
define('CHANGES[Title[en]]', M4_COMPANY_NAME M4_PRODUCT_NAME Release Notes)dnl
define('CHANGES[Title[ru]]', История изменений M4_COMPANY_NAME M4_PRODUCT_NAME)dnl
define('CHANGES[More[en]]', 'and a little more...')dnl
define('CHANGES[More[ru]]', 'и многое другое...')dnl
define('CHANGES[Url]', https://github.com/ONLYOFFICE/DesktopEditors/blob/master/CHANGELOG.md'#'M4_ANCHOR_VERSION)dnl
changequote({{,}})dnl
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>defn(CHANGES[Title[M4_L10N]])</title>
    <style type="text/css">
        body {
            background:white;
            font: 12px "Lucida Grande", "Lucida Sans Unicode", Verdana, Lucida, Helvetica, sans-serif;
        }
        h1, h2, h3 {
            color: #000000;
            font-family: "Helvetica";
            font-weight: normal;
            font-style: normal;
        }
        h1 {
            font-size: 18px;
        }
        h2 {
            font-size: 16px;
        }
        h3 {
            font-size: 14px;
        }
        .releasedate {
            color: #888;
            font-size: medium;
        }
        .version {
            border-bottom: 1px solid #cfcfcf;
        }
    </style>
</head>
<body>
    <div class="version">
        <h1>M4_COMPANY_NAME M4_PRODUCT_NAME M4_PRODUCT_VERSION<span class="releasedate"> - M4_RELEASE_DATE</span></h1>
include(win-linux/package/windows/update/changes/M4_PRODUCT_VERSION/M4_L10N.html)
        <div style="margin:0 0 20px 0;"><a href="defn({{CHANGES[Url]}})" target="_blank">defn({{CHANGES[More[M4_L10N]]}})</a></div>
    </div>
</body>
</html>
