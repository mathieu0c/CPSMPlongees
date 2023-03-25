[UninstallDelete]
Type: filesandordirs; Name: "{app}"


[Code]
function CreateFile(): boolean;
var
  fileName : string;
  lines : TArrayOfString;
begin
  Result := true;
  fileName := ExpandConstant('{app}\UPDATED.TAG');
  SetArrayLength(lines, 1);
  lines[0] := '42';
  Result := SaveStringsToFile(filename,lines,true);
  exit;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if  CurStep=ssPostInstall then
    begin
         CreateFile();
    end
end;
