#read input file
fin = open("DesktopEditorsPortable.evb", "rt")
#read file contents to string
data = fin.read()
lines = fin.readlines()
#replace all occurrences of the required string

#close the input file
fin.close()
#open the input file in write mode
fin = open("DesktopEditorsPortable.evb", "wt")
data = data.replace('<ActiveX>False</ActiveX>', '')
data = data.replace('<ActiveXInstall>False</ActiveXInstall>', '')
data = data.replace('<Action>0</Action>', '')
data = data.replace('<OverwriteDateTime>False</OverwriteDateTime>', '')
data = data.replace('<OverwriteAttributes>False</OverwriteAttributes>', '')
data = data.replace('<PassCommandLine>False</PassCommandLine>', '')
data = data.replace('<HideFromDialogs>0</HideFromDialogs>', '')
#overrite the input file with the resulting data
fin.write(data)
#close the file
fin.close()