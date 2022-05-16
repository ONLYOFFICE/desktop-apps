
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

function checkVCRedist2013(): Boolean;
var
  upgradecode: String;
begin

  if Is64BitInstallMode then
    upgradecode := '{20400CF0-DE7C-327E-9AE4-F0F38D9085F8}' //x64
  else
    upgradecode := '{B59F5BF1-67C8-3802-8E59-2CE551A39FC5}'; //x86

  Result :=  msiproductupgrade(upgradecode, '12');
end;

function checkVCRedist2022(): Boolean;
var
  upgradecode: String;
begin

  if Is64BitInstallMode then
    upgradecode := '{36F68A90-239C-34DF-B58C-64B30153CE35}' //x64
  else
    upgradecode := '{65E5BD06-6392-3027-8C26-853107D3CF1A}'; //x86

  Result :=  msiproductupgrade(upgradecode, '14');
end;
