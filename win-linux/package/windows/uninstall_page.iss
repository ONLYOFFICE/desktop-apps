
[CustomMessages]
en.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
cs.UninstallPageLabel=Stisknutím Odinstalovat pokračujte v odinstalování.
sk.UninstallPageLabel=Stlačením Odinštalovať pokračujte v odinštalácii.
ru.UninstallPageLabel=Нажмите кнопку «Удалить», чтобы приступить к удалению.
de.UninstallPageLabel=Klicken Sie auf Deinstallieren, um mit der Deinstallation fortzufahren.
fr.UninstallPageLabel=Appuyez sur Désinstaller pour effectuer la désinstallation.
es.UninstallPageLabel=Presione Desinstalar para continuar con la desinstalación.
it_IT.UninstallPageLabel=Scegliere Disinstalla per procedere con la disinstallazione.
pt_BR.UninstallPageLabel=Clique em Desinstalar para prosseguir com a desinstalação.
pl.UninstallPageLabel=Kliknij Odinstaluj, aby rozpocząć proces odinstalowywania.
lo.UninstallPageLabel=ກົດປຸ່ມຖອນການຕິດຕັ້ງ ເພື່ອດຳເນີນການຖອນການຕິດຕັ້ງ.
nl.UninstallPageLabel=Druk op Verwijderen om verder te gaan met het verwijderen.

en.UninstallOptionClearData=Clear all user settings and application cached data
cs.UninstallOptionClearData=Vymažte všechna uživatelská nastavení a data uložená v paměti
sk.UninstallOptionClearData=Vymazať všetky používateľské nastavenia a údaje uložené vo vyrovnávacej pamäti
ru.UninstallOptionClearData=Очистить все пользовательские настройки и кэш приложения
de.UninstallOptionClearData=Alle Benutzereinstellungen und zwischengespeicherten Daten der Anwendung löschen
fr.UninstallOptionClearData=Effacer tous les paramètres utilisateur et les données en cache de l'application
es.UninstallOptionClearData=Eliminar todos los ajustes de usuario y datos en caché de la aplicación
it_IT.UninstallOptionClearData=Cancella le impostazioni utente e i dati memorizzati nella cache dell’applicazione
pt_BR.UninstallOptionClearData=Limpar todas definições de usuário e dados salvos do programa
pl.UninstallOptionClearData=Usuń wszystkie ustawienia użytkownika oraz dane pamięci podręcznej aplikacji
lo.UninstallOptionClearData=ລືບຂໍ້ມູນທີ່ເຫັບໄວ້ໃນແອັບພລິເຄຊັ່ນ ແລະ ລືບການຕັ້ງຄ່າຜູ້ໃຊ້ທັ້ງໝົດ
nl.UninstallOptionClearData=Alle gebruikersinstellingen en cachegegevens van toepassingen wissen


[Code]
var
  IsClearData: Boolean;

procedure InitializeUninstallProgressForm();
var
  PageText: TNewStaticText;
  PageNameLabel: string;
  PageDescriptionLabel: string;
  CancelButtonEnabled: Boolean;
  CancelButtonModalResult: Integer;
  CheckBox: TNewCheckBox;
  UninstallFirstPage: TNewNotebookPage;
  UninstallNextButton: TNewButton;
begin
  IsClearData := False;

  if not UninstallSilent then
  begin
    { Create the first page and make it active }
    UninstallFirstPage := TNewNotebookPage.Create(UninstallProgressForm);
    UninstallFirstPage.Notebook := UninstallProgressForm.InnerNotebook;
    UninstallFirstPage.Parent := UninstallProgressForm.InnerNotebook;
    UninstallFirstPage.Align := alClient;

    PageText := TNewStaticText.Create(UninstallProgressForm);
    PageText.Parent := UninstallFirstPage;
    PageText.Top := UninstallProgressForm.StatusLabel.Top;
    PageText.Left := UninstallProgressForm.StatusLabel.Left;
    PageText.Width := UninstallProgressForm.StatusLabel.Width;
    PageText.Height := UninstallProgressForm.StatusLabel.Height;
    PageText.AutoSize := False;
    PageText.ShowAccelChar := False;
    PageText.Caption := ExpandConstant('{cm:UninstallPageLabel}');

    CheckBox := TNewCheckBox.Create(UninstallProgressForm);
    CheckBox.Parent := UninstallFirstPage;
    CheckBox.Top := PageText.Top + ScaleY(50);
    CheckBox.Left := PageText.Left;
    CheckBox.Width := UninstallProgressForm.Width;
    CheckBox.Caption := ' ' + ExpandConstant('{cm:UninstallOptionClearData}');

    UninstallProgressForm.InnerNotebook.ActivePage := UninstallFirstPage;

    PageNameLabel := UninstallProgressForm.PageNameLabel.Caption;
    PageDescriptionLabel := UninstallProgressForm.PageDescriptionLabel.Caption;

    { Create the second page }

    UninstallNextButton := TNewButton.Create(UninstallProgressForm);
    UninstallNextButton.Parent := UninstallProgressForm;
    UninstallNextButton.Left := UninstallProgressForm.CancelButton.Left - UninstallProgressForm.CancelButton.Width - ScaleX(10);
    UninstallNextButton.Top := UninstallProgressForm.CancelButton.Top;
    UninstallNextButton.Width := UninstallProgressForm.CancelButton.Width;
    UninstallNextButton.Height := UninstallProgressForm.CancelButton.Height;
    UninstallNextButton.Caption := ExpandConstant('{cm:Uninstall}');
    { Make the "Uninstall" button break the ShowModal loop }
    UninstallNextButton.ModalResult := mrOK;

    UninstallNextButton.TabOrder := UninstallProgressForm.CancelButton.TabOrder;
    UninstallProgressForm.CancelButton.TabOrder := UninstallNextButton.TabOrder + 1;

    { Run our wizard pages } 
    //UpdateUninstallWizard;
    CancelButtonEnabled := UninstallProgressForm.CancelButton.Enabled
    UninstallProgressForm.CancelButton.Enabled := True;
    CancelButtonModalResult := UninstallProgressForm.CancelButton.ModalResult;
    UninstallProgressForm.CancelButton.ModalResult := mrCancel;

    if UninstallProgressForm.ShowModal = mrCancel then Abort;

    UninstallNextButton.Enabled := False;
    IsClearData := CheckBox.State = cbChecked;

    { Restore the standard page payout }
    UninstallProgressForm.CancelButton.Enabled := CancelButtonEnabled;
    UninstallProgressForm.CancelButton.ModalResult := CancelButtonModalResult;

    UninstallProgressForm.PageNameLabel.Caption := PageNameLabel;
    UninstallProgressForm.PageDescriptionLabel.Caption := PageDescriptionLabel;

    UninstallProgressForm.InnerNotebook.ActivePage := UninstallProgressForm.InstallingPage;
  end;
end;