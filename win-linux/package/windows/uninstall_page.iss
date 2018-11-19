
[CustomMessages]
en.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
cs.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
sk.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
ru.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
de.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
fr.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
es.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
it_IT.UninstallPageLabel=Press Uninstall to proceed with uninstallation.
pt_BR.UninstallPageLabel=Press Uninstall to proceed with uninstallation.

en.UninstallOptionClearData=Remove user settings and application cashed data
cs.UninstallOptionClearData=Remove user settings and application cashed data
sk.UninstallOptionClearData=Remove user settings and application cashed data
ru.UninstallOptionClearData=Remove user settings and application cashed data
de.UninstallOptionClearData=Remove user settings and application cashed data
fr.UninstallOptionClearData=Remove user settings and application cashed data
es.UninstallOptionClearData=Remove user settings and application cashed data
it_IT.UninstallOptionClearData=Remove user settings and application cashed data
pt_BR.UninstallOptionClearData=Remove user settings and application cashed data


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
    UninstallNextButton.Caption := 'Uninstall';
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