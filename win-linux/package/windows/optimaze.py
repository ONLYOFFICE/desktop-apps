import re



try:
    fin = open("DesktopEditorsPortable.evb", "rt")
    data = fin.read()
    fin.close()
    fin = open("DesktopEditorsPortable.evb", "wt")
    attributes = [
    '<ActiveX>False</ActiveX>',
    '<ActiveXInstall>False</ActiveXInstall>',
    '<Action>0</Action>',
    '<OverwriteDateTime>False</OverwriteDateTime>',
    '<OverwriteAttributes>False</OverwriteAttributes>',
    '<PassCommandLine>False</PassCommandLine>',
    '<HideFromDialogs>0</HideFromDialogs>',
    ]
    for i in attributes:
        data = data.replace(i, '')
    data = re.sub(r'\n\s*\n', '\n', data, flags=re.MULTILINE)
    fin.write(data)
    fin.close()
except FileNotFoundError:
    exit(2)
