<html>
    <body>
ifdef(`M4_EXE_URI',
`       <p>Win M4_WIN_ARCH-bit installer
            <a href="https://M4_S3_BUCKET.s3-eu-west-1.amazonaws.com/M4_EXE_URI">exe</a>
        </p>',)

ifdef(`M4_EXE_UPDATE_URI',
`       <p>Win M4_WIN_ARCH-bit update installer
            <a href="https://M4_S3_BUCKET.s3-eu-west-1.amazonaws.com/M4_EXE_UPDATE_URI">exe</a>
        </p>',)

ifdef(`M4_EXE_XP_URI',
`       <p>Win XP M4_WIN_ARCH-bit installer
            <a href="https://M4_S3_BUCKET.s3-eu-west-1.amazonaws.com/M4_EXE_XP_URI">exe</a>
        </p>',)

ifdef(`M4_ZIP_URI',
`        <p>Win M4_WIN_ARCH-bit archive
            <a href="https://M4_S3_BUCKET.s3-eu-west-1.amazonaws.com/M4_ZIP_URI">zip</a>
        </p>',)

ifdef(`M4_ZIP_XP_URI',
`        <p>Win XP M4_WIN_ARCH-bit archive
            <a href="https://M4_S3_BUCKET.s3-eu-west-1.amazonaws.com/M4_ZIP_XP_URI">zip</a>
        </p>',)
    </body>
</html>
