changequote()changequote(`,`)dnl
changecom(`<!--`,`-->`)dnl
define(`CHANGES_TITLE[en]`,format(`%s Release Notes`,M4_COMPANY_NAME M4_PRODUCT_NAME))dnl
define(`CHANGES_TITLE[ru]`,format(`История изменений %s`,M4_COMPANY_NAME M4_PRODUCT_NAME))dnl
define(`CHANGES_VERSION`,patsubst(M4_PACKAGE_VERSION,`\(\.\w+\)$`))dnl
define(`CHANGES_HEADING`,M4_COMPANY_NAME M4_PRODUCT_NAME CHANGES_VERSION)dnl
define(`CHANGES_RELEASE_DATE[en]`,esyscmd(echo -n $(LANG=en_US.UTF-8 date -u -d @M4_BUILD_TIMESTAMP "+%b %e&comma; %Y")))dnl
define(`CHANGES_RELEASE_DATE[ru]`,esyscmd(echo -n $(LANG=ru_RU.UTF-8 date -u -d @M4_BUILD_TIMESTAMP "+%e %b %Y")))dnl
define(`CHANGES_BODY_FILE`,esyscmd(echo -n $(dirname "__file__"))/changes/CHANGES_VERSION/L10N.html)dnl
define(`CHANGES_BODY`,patsubst(sinclude(CHANGES_BODY_FILE),`^`,`        `))dnl
define(`CHANGES_MORE[en]`,`and a little more...`)dnl
define(`CHANGES_MORE[ru]`,`и многое другое...`)dnl
define(`CHANGES_LINK_CHANGELOG`,format(`https://github.com/ONLYOFFICE/DesktopEditors/blob/master/CHANGELOG.md#%s`,patsubst(CHANGES_VERSION,`\.`)))dnl
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>defn(CHANGES_TITLE[L10N])</title>
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
        <h1>CHANGES_HEADING<span class="releasedate"> - defn(CHANGES_RELEASE_DATE[L10N])</span></h1>
CHANGES_BODY
        <div style="margin:0 0 20px 0;"><a href="CHANGES_LINK_CHANGELOG" target="_blank">defn(CHANGES_MORE[L10N])</a></div>
    </div>
</body>
</html>
