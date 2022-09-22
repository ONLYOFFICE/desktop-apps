Function RegistryExist(Path)
  On Error Resume Next

  Set objShell = CreateObject("WScript.Shell")

  RegistryPath = objShell.RegRead(Path)
  Err_Number = err.number
  On Error GoTo 0

  If Err_Number <> 0 Then
    RegistryExist = False
  Else
    RegistryExist = True
  End If
End Function


Function UninstallOlderVersion
  On Error Resume Next

  Set objShell = CreateObject("WScript.Shell")
  
  Dim Button, regPath

  regPath = "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\" + Session.Property("ProductName") + "_is1\UninstallString"

  If (RegistryExist(regPath) = True) Then
    RegistryPath = objShell.RegRead(regPath)
    If Session.Property("UILevel") <> 2 Then 
      Button = MsgBox(Session.Property("UNINSTALL_PREV_MSGBOX"), 1, Session.Property("Setup") + " " + Session.Property("ProductName"))
    End If
    If Button = 1 Then
      objShell.Run RegistryPath, 0, True
    ElseIf Session.Property("UILevel") = 2 Then
      objShell.Run RegistryPath + " /VERYSILENT", 0, True
    Else 
      Session.Property("UninstallOlderVersion") = "1"
    End If
  End If
End Function

Function SetLocaleProperty
  On Error Resume Next

  Set objShell = CreateObject("WScript.Shell")

  Dim objDictionary
  Dim regPath
  Dim locale

  regPath = "HKEY_LOCAL_MACHINE\SOFTWARE\" + Session.Property("MANUFACTURER_INSTALL_FOLDER") + "\" + Session.Property("PRODUCT_INSTALL_FOLDER") + "\locale"

  If (RegistryExist(regPath) = True) Then
    locale = objShell.RegRead(regPath)
  Else
    Set objDictionary = CreateObject("Scripting.Dictionary")
    objDictionary.CompareMode = vbTextCompare
    objDictionary.Add "1052", "sq"
    objDictionary.Add "1025", "ar"
    objDictionary.Add "1026", "bg"
    objDictionary.Add "1027", "ca"
    objDictionary.Add "2052", "zh-CN"
    objDictionary.Add "1028", "zh-CN"
    objDictionary.Add "1050", "hr"
    objDictionary.Add "1029", "cs"
    objDictionary.Add "1030", "da"
    objDictionary.Add "1043", "nl"
    objDictionary.Add "1033", "en"
    objDictionary.Add "1065", "fa"
    objDictionary.Add "1035", "fi"
    objDictionary.Add "1036", "fr"
    objDictionary.Add "1031", "de"
    objDictionary.Add "1032", "el"
    objDictionary.Add "1037", "he"
    objDictionary.Add "1038", "hr"
    objDictionary.Add "1039", "is"
    objDictionary.Add "1057", "id"
    objDictionary.Add "1040", "it"
    objDictionary.Add "1041", "ja"
    objDictionary.Add "1087", "kk"
    objDictionary.Add "1042", "ko"
    objDictionary.Add "1044", "nb"
    objDictionary.Add "2068", "nn"
    objDictionary.Add "1045", "pl"
    objDictionary.Add "2070", "pt-PT"
    objDictionary.Add "1049", "ru"
    objDictionary.Add "3098", "sr"
    objDictionary.Add "2074", "sr"
    objDictionary.Add "1051", "sk"
    objDictionary.Add "1060", "sl"
    objDictionary.Add "3082", "es"
    objDictionary.Add "1053", "sv"
    objDictionary.Add "1055", "tr"
    objDictionary.Add "1058", "uk"
    objDictionary.Add "1077", "zu"

    locale = objDictionary(Session.Property("ProductLanguage"))
  End If

  Session.Property("LOCALE") = locale
End Function

Function SetCustomPath
  On Error Resume Next

  Dim origPath
  Dim customPath
  Dim tokens

  origPath = Session.Property("APPDIR")
  tokens = Split(origPath, "\")

  Dim index
  Dim parentLenght
  If Session.Property("UILevel") = 2 Then parentLength = UBound(tokens) - 1 Else parentLength = UBound(tokens) - 2
  customPath = ""

  For index = 0 To parentLength
      If Not(tokens(index)) = Empty Then
      customPath = customPath & tokens(index) & "\"
    End If
  Next

  customPath = customPath & "MediaViewer"

  Session.Property("CUSTOM_PATH") = customPath
End Function
