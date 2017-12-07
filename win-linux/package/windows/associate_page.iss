
#if !defined(ASSC_APP_NAME)
# define ASSC_APP_NAME 'ONLYOFFICE'
#endif

[Setup]
ChangesAssociations=true


[CustomMessages]

en.AssociateDescription =Associate office document file types with {#ASSC_APP_NAME}
cs.AssociateDescription =Přiřadit typy souborů kancelářských dokumentů {#ASSC_APP_NAME}
sk.AssociateDescription =Priradiť typy súborov kancelárskych dokumentov {#ASSC_APP_NAME}
ru.AssociateDescription =Ассоциировать типы файлов офисных документов с {#ASSC_APP_NAME}
de.AssociateDescription =Verknüpfung von Office-Dokumenten mit {#ASSC_APP_NAME}
fr.AssociateDescription =Associer les fichiers office avec {#ASSC_APP_NAME}
es.AssociateDescription =Asociar tipos de archivos de documentos de oficina con {#ASSC_APP_NAME}
;it.AssociateDescription =Associare i tipi di file video/audio ad {#ASSC_APP_NAME}

en.AssociateCaption =File Associations
cs.AssociateCaption =Asociace souboru
sk.AssociateCaption =Asociácia súboru
ru.AssociateCaption =Ассоциации файлов
de.AssociateCaption =Dateiassoziationen
fr.AssociateCaption =Associations de fichiers
es.AssociateCaption =Asociaciones de archivos
;it.AssociateCaption =Associazioni dei file

en.AssociateDont =Do not associate
cs.AssociateDont =Neasociováno
sk.AssociateDont =Neasociované
ru.AssociateDont =Не ассоциировать
de.AssociateDont =Nicht assoziieren
fr.AssociateDont =Ne pas associer
es.AssociateDont =No asociar
;it.AssociateDont =Non associare

en.AssociateAll =Associate all
cs.AssociateAll =Asociovat vše
sk.AssociateAll =Asociovať všetko
ru.AssociateAll =Ассоциировать все
de.AssociateAll =Alle assoziieren
fr.AssociateAll =Associer tous
es.AssociateAll =Asociar todo
;it.AssociateAll =Associare tutto

en.AssociateSel =Associate selected
cs.AssociateSel =Vybraná asociace
sk.AssociateSel =Vybraná asociácia
ru.AssociateSel =Ассоциировать выбранные
de.AssociateSel =Ausgewählte assoziieren
fr.AssociateSel =Associer sélectionnés
es.AssociateSel =Asociar seleccionado
;it.AssociateSel =Associare selezione

en.AssociateAudio =File types
cs.AssociateAudio =Typy souborů
sk.AssociateAudio =Typy súborov
ru.AssociateAudio =Типы файлов
de.AssociateAudio =Dateitypen
fr.AssociateAudio =Types de fichiers
es.AssociateAudio =Tipos de archivos
;it.AssociateAudio =Tipi di file

en.extMSWord =Microsoft Word Document
cs.extMSWord =Microsoft Word Dokument
sk.extMSWord =Microsoft Word Dokument
ru.extMSWord =Документ Microsoft Word
de.extMSWord =Microsoft Word Dokument
fr.extMSWord =Document Microsoft Word
es.extMSWord =Documento de Microsoft Word

en.extMSExcel =Microsoft Excel Workbook
cs.extMSExcel =Microsoft Excel Sešit
sk.extMSExcel =Microsoft Excel Zošit
ru.extMSExcel =Книга Microsoft Excel
de.extMSExcel =Microsoft Excel Arbeitsmappe
fr.extMSExcel =Classeur Microsoft Excel
es.extMSExcel =Libro de Microsoft Excel

en.extMSPresentation =Microsoft PowerPoint Presentation
cs.extMSPresentation =Microsoft PowerPoint Prezentace
sk.extMSPresentation =Microsoft PowerPoint Prezentácia
ru.extMSPresentation =Презентация Microsoft PowerPoint
de.extMSPresentation =Microsoft PowerPoint Präsentation
fr.extMSPresentation =Présentation Microsoft PowerPoint
es.extMSPresentation =Presentación de PowerPoint Microsoft

en.extMSSlideshow =Microsoft PowerPoint Slideshow
cs.extMSSlideshow =Microsoft PowerPoint Slideshow
sk.extMSSlideshow =Microsoft PowerPoint Slideshow
ru.extMSSlideshow =Слайдшоу Microsoft PowerPoint
de.extMSSlideshow =Microsoft PowerPoint Slideshow
fr.extMSSlideshow =Diaporama Microsoft PowerPoint
es.extMSSlideshow =Presentación de Microsoft PowerPoint

en.extODT =OpenDocument Text Document
cs.extODT =Dokumenty OpenDocument
sk.extODT =Dokumenty OpenDocument
ru.extODT =Текстовый документ OpenDocument
de.extODT =OpenDocument Textdokument
fr.extODT =Document OpenDocument Texte
es.extODT =Documento de texto de OpenDocument

en.extODS =OpenDocument Spreadsheet
cs.extODS =Sešit OpenDocument
sk.extODS =Zošit OpenDocument
ru.extODS =Электронная таблица OpenDocument
de.extODS =OpenDocument Tabelle
fr.extODS =Classeur OpenDocument
es.extODS =Hoja de cálculo de OpenDocument

en.extODP =OpenDocument Presentation
cs.extODP =Prezentace OpenDocument
sk.extODP =Prezentácia OpenDocument
ru.extODP =Презентация OpenDocument
de.extODP =OpenDocument  Präsentation
fr.extODP =Présentation OpenDocument
es.extODP =Presentación de OpenDocument

en.defprogAppDescription=Free desktop office suite for document editing and collaboration
ru.defprogAppDescription=Бесплатный десктопный офисный пакет для редактирования документов и совместной работы
de.defprogAppDescription=Kostenlose Desktop-Office-Suite für Dokumentenbearbeitung und Zusammenarbeit
fr.defprogAppDescription=Suite bureautique d'applications de bureau gratuite pour l'édition de documents et la collaboration
es.defprogAppDescription=Paquete desktop de oficina gratuito para edición de documentos y colaboración

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

procedure initExtensions;
var
  prefix: string;
begin
  SetArrayLength(AudioExts, 16);
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
//  AudioExts[12] := 'TXT';
  AudioExts[12] := 'CSV';
  AudioExts[13] := 'PDF';
  AudioExts[14] := 'DJVU';
  AudioExts[15] := 'XPS';
  
  SetArrayLength(ExtensionRegistryInfo,  GetArrayLength(AudioExts));

  prefix := 'ASC.';

  ExtensionRegistryInfo[0]  := prefix + 'Document.1:'   + ExpandConstant('{cm:extMSWord}')          + ':' + '11';
  ExtensionRegistryInfo[1]  := prefix + 'Document.12:'  + ExpandConstant('{cm:extMSWord}')          + ':' + '7';
  ExtensionRegistryInfo[2]  := prefix + 'Sheet.1:'      + ExpandConstant('{cm:extMSExcel}')         + ':' + '16';
  ExtensionRegistryInfo[3]  := prefix + 'Sheet.12:'     + ExpandConstant('{cm:extMSExcel}')         + ':' + '10';
  ExtensionRegistryInfo[4]  := prefix + 'Show.1:'       + ExpandConstant('{cm:extMSPresentation}')  + ':' + '1';
  ExtensionRegistryInfo[5]  := prefix + 'Show.12:'      + ExpandConstant('{cm:extMSPresentation}')  + ':' + '9';
  ExtensionRegistryInfo[6]  := prefix + 'SlideShow.1:'  + ExpandConstant('{cm:extMSSlideshow}')     + ':' + '2';
  ExtensionRegistryInfo[7]  := prefix + 'SlideShow.12:' + ExpandConstant('{cm:extMSSlideshow}')     + ':' + '8';
  ExtensionRegistryInfo[8]  := prefix + 'Document.2:'   + ExpandConstant('{cm:extODT}')             + ':' + '12';
  ExtensionRegistryInfo[9]  := prefix + 'Sheet.2:'      + ExpandConstant('{cm:extODS}')             + ':' + '17';
  ExtensionRegistryInfo[10] := prefix + 'Show.2:'       + ExpandConstant('{cm:extODP}')             + ':' + '3';
  ExtensionRegistryInfo[11] := prefix + 'Rtf:'                                                      + ':' + '13';
  //ExtensionRegistryInfo[12] := prefix + 'Txt:'                                                      + ':' + '14';
  ExtensionRegistryInfo[12] := prefix + 'Csv:'                                                      + ':' + '18';
  ExtensionRegistryInfo[13] := prefix + 'Pdf:'                                                      + ':' + '5';
  ExtensionRegistryInfo[14] := prefix + 'DjVu:'                                                     + ':' + '4';
  ExtensionRegistryInfo[15] := prefix + 'Xps:'                                                      + ':' + '6';
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

procedure InitializeAssociatePage;
var
  associatePage: TWizardPage;
  lblAudio: TLabel;
  i: Integer;
begin
  initExtensions();

  associatePage := CreateCustomPage(wpSelectTasks, CustomMessage('AssociateCaption'), CustomMessage('AssociateDescription'));

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

procedure AddToDefaultPrograms;
var
  i: integer;
  argsArray: TArrayOfString;
begin
    RegWriteStringValue(HKEY_LOCAL_MACHINE, '{#APP_REG_PATH}\Capabilities', 'ApplicationDescription', ExpandConstant('{cm:defprogAppDescription}'));
    RegWriteStringValue(HKEY_LOCAL_MACHINE, '{#APP_REG_PATH}\Capabilities', 'ApplicationIcon', ExpandConstant('"{app}\{#NAME_EXE_OUT},0"'));
    RegWriteStringValue(HKEY_LOCAL_MACHINE, '{#APP_REG_PATH}\Capabilities', 'ApplicationName', '{#sAppName}');

    for i := 0 to GetArrayLength(AudioExts) - 1 do begin
      Explode(argsArray, ExtensionRegistryInfo[i],':');
      RegWriteStringValue(HKEY_LOCAL_MACHINE, '{#APP_REG_PATH}\Capabilities\FileAssociations', '.' + LowerCase(AudioExts[i]), argsArray[0]);
    end;

    RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\RegisteredApplications', 'DesktopEditors', '{#APP_REG_PATH}\Capabilities');
end;

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

      // checking existance is temporary locked to rewrite new icons indexes
      //if not RegKeyExists(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0]) then begin
        if Length(argsArray[1]) <> 0 then
          RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0], '', argsArray[1]);

        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0] + '\DefaultIcon', '', ExpandConstant('{app}\{#iconsExe},' + argsArray[2]));
        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0] + '\shell\open\command', '', ExpandConstant('"{app}\{#iconsExe}" "%1"'));
      //end;

      ext := LowerCase(AudioExts[i]);

      if isAssociateExtension(i) then
      begin
        if not RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '') then begin
          RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '', argsArray[0])
        end else
          RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '', progId1);

        if not RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext + '\OpenWithProgids', argsArray[0]) then
          RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext + '\OpenWithProgids', argsArray[0], '');
        
        if RegValueExists(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '') then 
          RegQueryStringValue(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '', progId2);

        if ((Length(progId2) <> 0) and (CompareText(progId2, argsArray[0]) <> 0)) or 
              ((Length(progId1) <> 0) and (CompareText(progId1, argsArray[0]) <> 0)) then 
        begin
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice');
          RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '', argsArray[0])
          //RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice', 'Progid', argsArray[0]);
        end;
      end else
      begin
        RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('Software\Classes\Applications\{#NAME_EXE_OUT}\shell\open\command'), '', ExpandConstant('"{app}\{#iconsExe}" "%1"'));
        RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('Software\Classes\.' + ext + '\OpenWithList\{#NAME_EXE_OUT}'), '', '');
      end;
    end;
  end;

  AddToDefaultPrograms;
end;

{
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
begin
    MsgBox(MemoDirInfo, mbInformation, MB_OK);
    Result:=MemoUserInfoInfo + NewLine + MemoDirInfo + NewLine + MemoTypeInfo + NewLine + MemoComponentsInfo + NewLine + MemoGroupInfo + NewLine + MemoTasksInfo;
end;
}

procedure UnassociateExtensions;
var
  i: Integer;
  argsArray: TArrayOfString;
  ext, str: string;
begin
  initExtensions();

  for  i := 0 to GetArrayLength(AudioExts) - 1 do
  begin     
    Explode(argsArray, ExtensionRegistryInfo[i],':');
    RegDeleteKeyIncludingSubkeys(HKEY_LOCAL_MACHINE, 'Software\Classes\' + argsArray[0]);

    ext := LowerCase(AudioExts[i]);
    RegDeleteValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext + '\OpenWithProgids', argsArray[0]);

    RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '', str);
    if CompareText(str, argsArray[0]) = 0 then
      RegDeleteValue(HKEY_LOCAL_MACHINE, 'Software\Classes\.' + ext, '');

    RegQueryStringValue(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '', str);
    if CompareText(str, argsArray[0]) = 0 then
      RegDeleteValue(HKEY_CURRENT_USER, 'Software\Classes\.' + ext, '');

    RegQueryStringValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice', 'Progid', str);
    if CompareText(str, argsArray[0]) = 0 then
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.' + ext + '\UserChoice');
  
    RegDeleteKeyIncludingSubkeys(HKEY_LOCAL_MACHINE, ExpandConstant('Software\Classes\Applications\{#NAME_EXE_OUT})'));
    RegDeleteKeyIncludingSubkeys(HKEY_LOCAL_MACHINE, ExpandConstant('Software\Classes\.' + ext + '\OpenWithList\{#NAME_EXE_OUT}'));
  end;

  RegDeleteValue(HKEY_LOCAL_MACHINE, 'Software\RegisteredApplications', 'DesktopEditors');
end;
