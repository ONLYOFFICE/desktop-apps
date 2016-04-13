[Code]

function GetRegnowBuyURL(const ProductCode: String) : String;
var
  InstallerName : String;
  AffiliateCode : String;
  DelimiterPos  : Integer;
begin
  Result        := '';
  AffiliateCode := '';
  InstallerName := ChangeFileExt( ExtractFileName(ExpandConstant('{srcexe}')), '' );
  DelimiterPos  := Pos('-', InstallerName);

  if ( DelimiterPos > 0 ) then
    AffiliateCode := Copy(InstallerName, DelimiterPos + 1, Length(InstallerName));

  if ( Length(AffiliateCode) > 0 ) then
  begin
    Result := 'http://www.regnow.com/softsell/nph-softsell.cgi?item=' + ProductCode;
    Result := Result + '&currency=USD&locale=EN&ss_short_order=true&styleid=15925';
    Result := Result + '&affiliate=' + AffiliateCode;
  end;
end;


function ReadInstallInfo(var DVDOpenMode : Integer; var Publisher, BuyURL,
                         PostInstallURL, FeedbackURL, ProlongURL,
                         HomePageURL, SupportURL, SupportMail : String): Boolean;
var
  Info: String;
  Mark1, Mark2, Mark3, Mark4, Mark5, Mark6, Mark7, Mark8, Mark9, Mark10 : Integer;
  f: TFileStream;
begin
  try
    f := TFileStream.Create(ExpandConstant('{srcexe}'), fmOpenRead or fmShareDenyNone);
  except
    Result := false;
    exit;
  end;

  SetLength(Info, 4000);
  try
    f.Seek(-6000, soFromEnd);
    f.ReadBuffer(Info, 4000);
  finally
    f.Free;
  end;

  //control mark
  Mark1 := Pos('******', Info);
  if ( Mark1 = 0 ) then
  begin
    Result := false;
    exit;
  end;

  Delete(Info, 1, Mark1 + 5);
  Mark1 := 1;

  Mark2 := Pos('*', Info);
  if ( Mark2 > 0 ) then
    Info[Mark2] := ' '
  else
  begin
    Result := false;
    exit;
  end;

  Mark3 := Pos('*', Info);
  if ( Mark3 > 0 ) then
    Info[Mark3] := ' '
  else
  begin
    Result := false;
    exit;
  end;

  Mark4 := Pos('*', Info);
  if ( Mark4 > 0 ) then
    Info[Mark4] := ' '
  else
  begin
    Result := false;
    exit;
  end;

  Mark5 := Pos('*', Info);
  Mark6 := 0;
  Mark7 := 0;
  Mark8 := 0;
  Mark9 := 0;
  Mark10 := 0;
  if ( Mark5 > 0 ) then
  begin
    Info[Mark5] := ' ';

    Mark6 := Pos('*', Info);
    if (Mark6 > 0) then
    begin
      Info[Mark6] := ' ';

      Mark7 := Pos('*', Info);
      if (Mark7 > 0) then
      begin
        Info[Mark7] := ' ';

        Mark8 := Pos('*', Info);
        if (Mark8 > 0) then
        begin
          Info[Mark8] := ' ';

          Mark9 := Pos('*', Info);
          if (Mark9 > 0) then
          begin
            Info[Mark9] := ' ';

            Mark10 := Pos('*', Info);
            if (Mark10 > 0) then
              Info[Mark10] := ' '
          end
        end
      end
    end
  end;

  DVDOpenMode := StrToIntDef( Trim( Copy(Info, Mark1, Mark2 - Mark1) ), 1 );
  Publisher   :=              Trim( Copy(Info, Mark2, Mark3 - Mark2) );
  BuyURL      :=              Trim( Copy(Info, Mark3, Mark4 - Mark3) );

  if (Mark5 > 0) and (Mark6 > 0) then
  begin
    PostInstallURL :=         Trim( Copy(Info, Mark4, Mark5 - Mark4) );
    FeedbackURL    :=         Trim( Copy(Info, Mark5, Mark6 - Mark5) );
    if (Mark7 > 0) then
      ProlongURL :=           Trim( Copy(Info, Mark6, Mark7 - Mark6) );

    if (Mark8 > 0) then
      HomePageURL :=           Trim( Copy(Info, Mark7, Mark8 - Mark7) );
    if (Mark9 > 0) then
      SupportURL :=           Trim( Copy(Info, Mark8, Mark9 - Mark8) );
    if (Mark10 > 0) then
      SupportMail :=           Trim( Copy(Info, Mark9, Mark10 - Mark9) );
  end;

  Result := true;
end;


function InternationalizeURL(const URL: String) : String;
var
  TmpURL : String;
  PointPos, SlashPos : Integer;
begin
  Result := URL;
  TmpURL := URL;

  if ( (ActiveLanguage = 'en') or (ActiveLanguage = 'ru') ) then
    exit;

  StringChangeEx(TmpURL, '\', '/', True);

  PointPos := Pos('.', TmpURL);
  if ( PointPos > 0 ) then
    Delete(TmpURL, 1, PointPos);

  SlashPos := Pos('/', TmpURL);
  if ( SlashPos > 0 ) then
    Insert(ActiveLanguage + Result[SlashPos+PointPos], Result, SlashPos + PointPos + 1);
end;


function InternationalizeURL2(const URL: String) : String;  // with Russian language
var
  TmpURL : String;
  PointPos, SlashPos : Integer;
begin
  Result := URL;
  TmpURL := URL;

  if ( ActiveLanguage = 'en' ) then
    exit;

  StringChangeEx(TmpURL, '\', '/', True);

  PointPos := Pos('.', TmpURL);
  if ( PointPos > 0 ) then
    Delete(TmpURL, 1, PointPos);

  SlashPos := Pos('/', TmpURL);
  if ( SlashPos > 0 ) then
    Insert(ActiveLanguage + Result[SlashPos+PointPos], Result, SlashPos + PointPos + 1);
end;


function InternationalizeURL3(const URL: String) : String;  // RU,DE,FR,ES,IT,JP
var
  TmpURL : String;
  PointPos, SlashPos : Integer;
begin
  Result := URL;
  TmpURL := URL;

  if ( (ActiveLanguage <> 'ru') and
       (ActiveLanguage <> 'de') and
       (ActiveLanguage <> 'fr') and
       (ActiveLanguage <> 'es') and
       (ActiveLanguage <> 'it') and
       (ActiveLanguage <> 'jp') ) then
    exit;

  StringChangeEx(TmpURL, '\', '/', True);

  PointPos := Pos('.', TmpURL);
  if ( PointPos > 0 ) then
    Delete(TmpURL, 1, PointPos);

  SlashPos := Pos('/', TmpURL);
  if ( SlashPos > 0 ) then
    Insert(ActiveLanguage + Result[SlashPos+PointPos], Result, SlashPos + PointPos + 1);
end;


procedure AddParameterToStr(var Str: String; ParamName, ParamValue: String);
begin
  if ( (Length(ParamName) > 0) and (Length(ParamValue) > 0) ) then
  begin
    if ( Length(Str) > 0 ) then
      Str := Str + '&';
    Str := Str + ParamName + '=' + ParamValue;
  end;
end;


function GoogleParametrizeURL(const URL, Source, Medium, Content, Campaign: String) : String;
var
  Parameters : String;
begin
  Result     := URL;
  Parameters := '';

  AddParameterToStr( Parameters, 'utm_source',   Source   );
  AddParameterToStr( Parameters, 'utm_medium',   Medium   );
  AddParameterToStr( Parameters, 'utm_content',  Content  );
  AddParameterToStr( Parameters, 'utm_campaign', Campaign );

  if ( Length(Parameters) > 0 ) then
    Result := Result + '?' + Parameters;
end;


function ParametrizeURL(const URL, ProgramID, TypeName, URLName, Publisher: String) : String;
var
  Parameters : String;
begin
  Result     := URL;
  Parameters := '';

  AddParameterToStr( Parameters, 'ProgID',    ProgramID );
  AddParameterToStr( Parameters, 'Type',      TypeName  );
  AddParameterToStr( Parameters, 'URL',       URLName   );
  AddParameterToStr( Parameters, 'Publisher', Publisher );

  if ( Length(Parameters) > 0 ) then
    Result := Result + '?' + Parameters;
end;


function UninstallerPath(const RootKey: Integer; const SubKeyName: String): String;
var
  Path: String; 	
begin
  if (RegQueryStringValue(RootKey, SubKeyName, 'Uninstall', Path)) then
  begin
    if (FileExists(Path)) then
      Result := Path
    else
      Result := '';
  end;
end;


function NeedUninstall(const RootKey: Integer; const SubKeyName: String): Boolean;
var
  Counter: Cardinal; //32-bit unsigned integer	
begin
  Counter := 1;

  if (RegQueryDWordValue(RootKey, SubKeyName, 'SharedCounter', Counter)) then
  begin
    Counter := Counter - 1;
    RegWriteDWordValue(RootKey, SubKeyName, 'SharedCounter', Counter);
  end;

  Result := (Counter <= 0);
end;
