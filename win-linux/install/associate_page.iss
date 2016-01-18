
[Setup]
ChangesAssociations=true

[CustomMessages]

en.AssociateDescription =Associate office document file types with %1
ru.AssociateDescription =Ассоциировать типы файлов офисных документов с %1
;de.AssociateDescription =Video- und Audiodateitypen mit AVS Media Player assoziieren
;fr.AssociateDescription =Associer des types de fichiers vidйo et audio а AVS Media Player
;es.AssociateDescription =Asociar tipos de archivos de vнdeo y audio con AVS Media Player
;it.AssociateDescription =Associare i tipi di file video/audio ad AVS Media Player

en.AssociateCaption =File Associations
ru.AssociateCaption =Ассоциации файлов
;de.AssociateCaption =Dateiassoziationen
;fr.AssociateCaption =Associations de fichiers
;es.AssociateCaption =Asociaciones de archivos
;it.AssociateCaption =Associazioni dei file

en.AssociateDont =Do not associate
ru.AssociateDont =Не ассоциировать
;de.AssociateDont =Nicht assoziieren
;fr.AssociateDont =Ne pas associer
;es.AssociateDont =No asociar
;it.AssociateDont =Non associare

en.AssociateAll =Associate all
ru.AssociateAll =Ассоциировать все
;de.AssociateAll =Alle assoziieren
;fr.AssociateAll =Associer tous
;es.AssociateAll =Asociar todo
;it.AssociateAll =Associare tutto

en.AssociateSel =Associate selected
ru.AssociateSel =Ассоциировать выбранные
;de.AssociateSel =Ausgewдhlte assoziieren
;fr.AssociateSel =Associer sйlectionnйs
;es.AssociateSel =Asociar seleccionado
;it.AssociateSel =Associare selezione

en.AssociateAudio =File types
ru.AssociateAudio =Типы файлов
;de.AssociateAudio =Audiodateitypen
;fr.AssociateAudio =Types de fichiers audio
;es.AssociateAudio =Tipos de archivos de audio
;it.AssociateAudio =Tipi di file audio

en.extMSWord =Microsoft Word Document
ru.extMSWord =Документ Microsoft Word

en.extMSExcel =Microsoft Excel Workbook
ru.extMSExcel =Книга Microsoft Excel

en.extMSPresentation =Microsoft PowerPoint Presentation
ru.extMSPresentation =Презентация Microsoft PowerPoint

en.extMSSlideshow =Microsoft PowerPoint Slideshow
ru.extMSSlideshow =Слайдшоу Microsoft PowerPoint

en.extODT =OpenDocument Text Document
ru.extODT =Текстовый документ OpenDocument

en.extODS =OpenDocument Spreadsheet
ru.extODS =Электронная таблица OpenDocument

en.extODP =OpenDocument Presentation
ru.extODP =Презентация OpenDocument

[Code]

var
  OnAudioClick: Boolean;
  ChlbAudio: TNewCheckListBox;
  AudioExtEnabled: Array of Boolean;
  AudioExts: Array of String;
  ExtensionRegistryInfo: array of string;
  AChecked: Boolean;

procedure Explode(var Dest: TArrayOfString; Text: String; Separator: String);
var
  i, p: Integer;
begin
  i := 0;
  repeat
    SetArrayLength(Dest, i+1);
    p := Pos(Separator,Text);
    if p > 0 then begin
      Dest[i] := Copy(Text, 1, p-1);
      Text := Copy(Text, p + Length(Separator), Length(Text));
      i := i + 1;
    end else begin
      Dest[i] := Text;
      Text := '';
    end;
  until Length(Text)=0;
end;

procedure initExensions;
var
  prefix: string;
begin
  SetArrayLength(AudioExts, 17);
  SetArrayLength(AudioExtEnabled,  GetArrayLength(AudioExts));

  AudioExts[0]  := 'DOC';
  AudioExts[1]  := 'DOCX';
  AudioExts[2]  := 'XLS';
  AudioExts[3]  := 'XLSX';
  AudioExts[4]  := 'PPT';
  AudioExts[5]  := 'PPTX';
  AudioExts[6]  := 'PPS';
  AudioExts[7]  := 'PPSX';
  AudioExts[8]  := 'ODT';
  AudioExts[9]  := 'ODS';
  AudioExts[10] := 'ODP';
  AudioExts[11] := 'RTF';
  AudioExts[12] := 'TXT';
  AudioExts[13] := 'CSV';
  AudioExts[14] := 'PDF';
  AudioExts[15] := 'DJVU';
  AudioExts[16] := 'XPS';
  
  SetArrayLength(ExtensionRegistryInfo,  GetArrayLength(AudioExts));
  
  prefix := 'ASC.';
  ExtensionRegistryInfo[0]  := prefix + 'Document.1:'   + ExpandConstant('{cm:extMSWord}')          + ':' + '7';
  ExtensionRegistryInfo[1]  := prefix + 'Document.1:'   + ExpandConstant('{cm:extMSWord}')          + ':' + '7';
  ExtensionRegistryInfo[2]  := prefix + 'Sheet.1:'      + ExpandConstant('{cm:extMSExcel}')         + ':' + '12';
  ExtensionRegistryInfo[3]  := prefix + 'Sheet.1:'      + ExpandConstant('{cm:extMSExcel}')         + ':' + '12';
  ExtensionRegistryInfo[4]  := prefix + 'Show.1:'       + ExpandConstant('{cm:extMSPresentation}')  + ':' + '1';
  ExtensionRegistryInfo[5]  := prefix + 'Show.1:'       + ExpandConstant('{cm:extMSPresentation}')  + ':' + '1';
  ExtensionRegistryInfo[6]  := prefix + 'SlideShow.1:'  + ExpandConstant('{cm:extMSSlideshow}')     + ':' + '2';
  ExtensionRegistryInfo[7]  := prefix + 'SlideShow.1:'  + ExpandConstant('{cm:extMSSlideshow}')     + ':' + '2';
  ExtensionRegistryInfo[8]  := prefix + 'Document.2:'   + ExpandConstant('{cm:extODT}')             + ':' + '8';
  ExtensionRegistryInfo[9]  := prefix + 'Sheet.2:'      + ExpandConstant('{cm:extODS}')             + ':' + '13';
  ExtensionRegistryInfo[10] := prefix + 'Show.2:'       + ExpandConstant('{cm:extODP}')             + ':' + '3';
  ExtensionRegistryInfo[11] := prefix + 'Rtf:'                                                      + ':' + '9';
  ExtensionRegistryInfo[12] := prefix + 'Txt:'                                                      + ':' + '10';
  ExtensionRegistryInfo[13] := prefix + 'Csv:'                                                      + ':' + '14';
  ExtensionRegistryInfo[14] := prefix + 'Pdf:'                                                      + ':' + '5';
  ExtensionRegistryInfo[15] := prefix + 'DjVu:'                                                     + ':' + '4';
  ExtensionRegistryInfo[16] := prefix + 'Xps:'                                                      + ':' + '6';
end;

procedure ChlbAudioClickCheck(Sender: TObject);
var
  i: Integer;
begin
  if not OnAudioClick then
  begin
    OnAudioClick := True;
    if ChlbAudio.Checked[2] then
    begin
      if not AChecked then
      begin
        AChecked := True;
        for i := 0 to GetArrayLength(AudioExts) - 1 do
        begin
          ChlbAudio.ItemEnabled[i + 3] := True;
          ChlbAudio.Checked[i + 3] := AudioExtEnabled[i];
        end;
      end
      else
      begin
        for i := 0 to GetArrayLength(AudioExts) - 1 do
          AudioExtEnabled[i] := ChlbAudio.Checked[i + 3];
      end;
    end
    else
    begin
      AChecked := False;
      for i := 0 to GetArrayLength(AudioExts) - 1 do
      begin
        ChlbAudio.ItemEnabled[i + 3] := False;
//        ChlbAudio.Checked[i + 3] := ArrAudio[i];
      end;
    end;
    OnAudioClick := False;
    ChlbAudio.Repaint;
  end;
end;

procedure InitializeWizard();
var
  associatePage: TWizardPage;
  lblAudio: TLabel;
  i: Integer;
begin
  initExensions();

  associatePage := CreateCustomPage(wpSelectTasks, ExpandConstant('{cm:AssociateCaption}'), ExpandConstant('{cm:AssociateDescription, {cm:AppName}}'));

  lblAudio          := TLabel.Create(associatePage);
  lblAudio.Parent   := associatePage.Surface;
  lblAudio.WordWrap := False;
  lblAudio.Caption  := ExpandConstant('{cm:AssociateAudio}');
  lblAudio.AutoSize := False;
  lblAudio.Width    := associatePage.SurfaceWidth;
  lblAudio.Left     := 0;
  lblAudio.Top      := 0;

  ChlbAudio         := TNewCheckListBox.Create(associatePage);
  ChlbAudio.Parent  := associatePage.Surface;
  ChlbAudio.Left    := 0;
  ChlbAudio.Top     := lblAudio.Top + lblAudio.Height + 4;
  ChlbAudio.Width   := associatePage.SurfaceWidth;
  ChlbAudio.Height  := associatePage.SurfaceHeight - ChlbAudio.Top - 4 - 3;

  ChlbAudio.AddRadioButton(ExpandConstant('{cm:AssociateDont}'), '', 0, False, True, nil);
  ChlbAudio.AddRadioButton(ExpandConstant('{cm:AssociateAll}'),  '', 0, False, True, nil);
  ChlbAudio.AddRadioButton(ExpandConstant('{cm:AssociateSel}'),  '', 0, True,  True, nil); 
  AChecked := True;

  for  i := 0 to GetArrayLength(AudioExts) - 1 do
  begin
    ChlbAudio.AddCheckBox(AudioExts[i], '', 1, False, True, False, False, nil);
    AudioExtEnabled[i] := True;
  end;

  OnAudioClick := False;
  ChlbAudio.OnClickCheck := @ChlbAudioClickCheck;

  ChlbAudio.Checked[1] := True;
  ChlbAudioClickCheck(ChlbAudio);

  //vc_desctopiconshow := True;
  //WizardForm.TasksList.OnClickCheck := @OnTasksListClickCheck;
end;

//----------

function isAssociateExtension(index: Integer): Boolean;
begin
  Result := ChlbAudio.Checked[1] or (ChlbAudio.Checked[2] and ChlbAudio.Checked[index + 3]);
end;

//----------

procedure DoPostInstall();
var
  i: Integer;
  ext, progId1, progId2: string;
  argsArray: TArrayOfString;
begin
  if ( not WizardSilent() ) then
  begin
    for  i := 0 to GetArrayLength(AudioExts) - 1 do
    begin     
      Explode(argsArray, ExtensionRegistryInfo[i],':');

      if not RegKeyExists(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0]) then begin
        if Length(argsArray[1]) <> 0 then
          RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0], '', argsArray[1]);
        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0] + '\DefaultIcon', '', ExpandConstant('{app}\{#iconsExe},' + argsArray[2]));
        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0] + '\shell\open\command', '', ExpandConstant('"{app}\{#NAME_EXE_OUT}" "%1"'));
      end;

      ext := LowerCase(AudioExts[i]);
      if not RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext + '\OpenWithProgids', argsArray[0]) then
        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext + '\OpenWithProgids', argsArray[0], '');

      if isAssociateExtension(i) then
      begin
        if not RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '') then begin
          RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '', argsArray[0])
        end else
          RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '', progId1);
        
        if RegValueExists(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '') then 
          RegQueryStringValue(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '', progId2);

        if ((Length(progId2) <> 0) and (CompareText(progId2, argsArray[0]) <> 0)) or 
              ((Length(progId1) <> 0) and (CompareText(progId1, argsArray[0]) <> 0)) then 
        begin
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice');
          RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice', 'Progid', argsArray[0]);
        end;

      end;
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then DoPostInstall();
end;

{
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
begin
    MsgBox(MemoDirInfo, mbInformation, MB_OK);
    Result:=MemoUserInfoInfo + NewLine + MemoDirInfo + NewLine + MemoTypeInfo + NewLine + MemoComponentsInfo + NewLine + MemoGroupInfo + NewLine + MemoTasksInfo;
end;
}

procedure CurUninstallStepChanged(CurStep: TUninstallStep);
var
  i: Integer;
  argsArray: TArrayOfString;
  ext, str: string;
begin
  if CurStep = usUninstall then
  begin
    initExensions();

    for  i := 0 to GetArrayLength(AudioExts) - 1 do
    begin     
      Explode(argsArray, ExtensionRegistryInfo[i],':');
      RegDeleteKeyIncludingSubkeys(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0]);

      ext := LowerCase(AudioExts[i]);
      RegDeleteValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext + '\OpenWithProgids', argsArray[0]);

      RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '', str);
      if CompareText(str, argsArray[0]) = 0 then
        RegDeleteValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '');

      RegQueryStringValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice', 'Progid', str);
      if CompareText(str, argsArray[0]) = 0 then
        RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice');
    end;
  end;
end;