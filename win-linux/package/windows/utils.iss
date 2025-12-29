
#include "stringversion.iss"
#include "msiproduct.iss"

[Code]

procedure DirectoryCopy(SourcePath, DestPath: string);
var
  FindRec: TFindRec;
  SourceFilePath: string;
  DestFilePath: string;
begin
  if FindFirst(SourcePath + '\*', FindRec) then begin
    try
      repeat
        if (FindRec.Name <> '.') and (FindRec.Name <> '..') then begin
          SourceFilePath := SourcePath + '\' + FindRec.Name;
          DestFilePath := DestPath + '\' + FindRec.Name;
          if FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY = 0 then begin
            if not FileCopy(SourceFilePath, DestFilePath, False) then
              Log(Format('Failed to copy %s to %s', [SourceFilePath, DestFilePath]));
          end else begin
            if DirExists(DestFilePath) or CreateDir(DestFilePath) then begin
              DirectoryCopy(SourceFilePath, DestFilePath);
            end else
              Log(Format('Failed to create %s', [DestFilePath]));
          end;
        end;
      until not FindNext(FindRec);
    finally
      FindClose(FindRec);
    end;
  end else begin
    Log(Format('Failed to list %s', [SourcePath]));
  end;
end;

function StartsWith(SubStr, S: String): Boolean;
begin
   Result := Pos(SubStr, S) = 1;
end;

function StringReplace(S, oldSubString, newSubString: String) : String;
var
  stringCopy : String;
begin
  stringCopy := S; //Prevent modification to the original string
  StringChange(stringCopy, oldSubString, newSubString);
  Result := stringCopy;
end;

function GetCommandlineParam(inParamName: String) : String;
var
   paramNameAndValue: String;
   i: Integer;
begin
   Result := '';

   for i:= 1 to ParamCount do begin
     paramNameAndValue := Lowercase(ParamStr(i));
     if StartsWith(inParamName, paramNameAndValue) then begin
       Result := StringReplace(paramNameAndValue, inParamName + ':', '');
       break;
     end;
   end;
end;

function CheckCommandlineParam(inpn: String) : Boolean;
var
   i: Integer;
begin
   Result := false;

   for i:= 1 to ParamCount do begin
     if Lowercase(inpn) = Lowercase(ParamStr(i)) then begin
       Result := true;
       break;
     end;
   end;
end;

function checkVCRedist(): Boolean;
var
  upgradecode: String;
begin

#if ARCH == "arm64"

  upgradecode := '{DC9BAE42-810B-423A-9E25-E4073F1C7B00}'; //arm64

  Result :=  msiproductupgrade(upgradecode, '14.32.31332.0');

#else
#ifndef _WIN_XP

  if Is64BitInstallMode then
    upgradecode := '{36F68A90-239C-34DF-B58C-64B30153CE35}' //x64
  else
    upgradecode := '{65E5BD06-6392-3027-8C26-853107D3CF1A}'; //x86

  Result :=  msiproductupgrade(upgradecode, '14.32.31332.0');

#else

  if Is64BitInstallMode then
    upgradecode := '{C146EF48-4D31-3C3D-A2C5-1E91AF8A0A9B}' //x64
  else
    upgradecode := '{F899BAD3-98ED-308E-A905-56B5338963FF}'; //x86

  Result :=  msiproductupgrade(upgradecode, '14.27.29114.0');

#endif
#endif

end;

function ReadBinFile(fileName: String; list: TStringList): Boolean;
var
  fs: TFileStream;
  buff: String;
  len: Word;
  ch: Char;
begin
  Result := False;
  if not FileExists(fileName) then
    Exit;
  list.Clear;
  try
    fs := TFileStream.Create(fileName, fmOpenRead);
  except
    Exit;
  end;
  while fs.Position < fs.Size do begin
    SetLength(buff, 1);
    try
      fs.ReadBuffer(buff, SizeOf(len));
    except
      fs.Free;
      Exit;
    end;
    len := Ord(buff[1]);
    SetLength(buff, len);
    try
      fs.ReadBuffer(buff, len * SizeOf(ch));
    except
      fs.Free;
      Exit;
    end;
    list.Add(buff);
  end;
  Result := True;
  fs.Free;
end;

procedure RemoveExtraFiles();
var
  i: Integer;
  appPath, path: String;
  files: TStringList;
begin
  files := TStringList.Create;
  appPath := ExpandConstant('{app}');
  if ReadBinFile(appPath + '\unins000.bin', files) then begin
    for i := 0 to files.Count - 1 do begin
      if DeleteFile(appPath + files[i]) then begin
        path := ExtractFileDir(files[i]);
        while (path <> '\') do begin
          if not RemoveDir(appPath + path) then
            break;
          path := ExtractFileDir(path);
        end;
      end;
    end;
  end;
  files.Free;
  DeleteFile(appPath + '\unins000.bin');
end;
