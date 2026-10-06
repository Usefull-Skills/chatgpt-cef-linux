#ifndef MyVersion
  #define MyVersion "0.8.0-dev"
#endif
#ifndef BrowserPayload
  #define BrowserPayload "..\build-windows\bin"
#endif

[Setup]
AppId={{CC2E8C3B-A028-43C0-A8AA-8D815C3F6A43}
AppName=Remote Commander Browser
AppVersion={#MyVersion}
AppVerName=Remote Commander Browser {#MyVersion}
AppPublisher=Remote Commander
CreateAppDir=no
Uninstallable=no
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist\installer
OutputBaseFilename=Remote-Commander-Browser-Setup-v{#MyVersion}
SetupIconFile={#BrowserSetupIcon}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
DisableWelcomePage=no
RestartApplications=no

[Files]
Source: "{#BrowserPayload}\*"; DestDir: "{tmp}\rc-browser-payload"; Flags: ignoreversion recursesubdirs createallsubdirs deleteafterinstall
Source: "..\scripts\install-windows.ps1"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall
Source: "..\assets\remote-commander-browser-logo.png"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall

[Run]
Filename: "{localappdata}\Programs\Remote Commander Browser\current\chatgpt-cef-v2.exe"; Description: "Open Remote Commander Browser"; Flags: nowait postinstall skipifsilent

[Code]
var
  PwshPath: String;
  BrowserInstallExitCode: Integer;

function FindPwsh: String;
begin
  Result := ExpandConstant('{pf64}\PowerShell\7\pwsh.exe');
  if not FileExists(Result) then Result := '';
end;

procedure EnsurePowerShell7;
var
  Msi, Url, Sha: String;
  ResultCode: Integer;
begin
  PwshPath := FindPwsh;
  if PwshPath <> '' then Exit;

  Url := 'https://github.com/PowerShell/PowerShell/releases/download/v7.6.6/PowerShell-7.6.6-win-x64.msi';
  Sha := '958838FF55091E1C8705D89EFED0CC7E8245A3A6EF6C0CCFAE20015227108AD8';
  Msi := ExpandConstant('{tmp}\PowerShell-7.6.6-win-x64.msi');
  WizardForm.StatusLabel.Caption := 'Downloading verified PowerShell 7 prerequisite...';
  DownloadTemporaryFile(Url, ExtractFileName(Msi), Sha, nil);

  if not ShellExec('runas', ExpandConstant('{sys}\msiexec.exe'),
      '/i "' + Msi + '" /qn /norestart', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    RaiseException('Could not elevate PowerShell prerequisite installation.');
  if (ResultCode <> 0) and (ResultCode <> 3010) then
    RaiseException('PowerShell installer failed with exit code ' + IntToStr(ResultCode) + '.');

  PwshPath := FindPwsh;
  if PwshPath = '' then RaiseException('PowerShell 7 installation completed but pwsh.exe was not found.');
end;

function GetPwshPath(Param: String): String;
begin
  Result := PwshPath;
end;

function GetInstallArguments(Param: String): String;
var
  InstallRoot, CompanionRoot, NoPublic: String;
begin
  InstallRoot := ExpandConstant('{param:RCInstallRoot|}');
  CompanionRoot := ExpandConstant('{param:RCCompanionRoot|}');
  NoPublic := ExpandConstant('{param:RCNoPublicIntegration|0}');

  Result := '-NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "' +
    ExpandConstant('{tmp}\install-windows.ps1') + '" -SourceDir "' +
    ExpandConstant('{tmp}\rc-browser-payload') + '" -VersionOverride "{#MyVersion}" -LogoPath "' +
    ExpandConstant('{tmp}\remote-commander-browser-logo.png') + '"';

  if InstallRoot <> '' then Result := Result + ' -InstallRoot "' + InstallRoot + '"';
  if CompanionRoot <> '' then Result := Result + ' -CompanionRoot "' + CompanionRoot + '"';
  if NoPublic = '1' then Result := Result + ' -NoPublicIntegration';
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  try
    EnsurePowerShell7;
  except
    Result := GetExceptionMessage;
  end;
end;

procedure BrowserInstallLog(const S: String; const Error, FirstLine: Boolean);
begin
  Log('Browser installer: ' + S);
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
  Started: Boolean;
begin
  if CurStep = ssPostInstall then
  begin
    ResultCode := 0;
    try
      Started := ExecAndLogOutput(PwshPath, GetInstallArguments(''), '', SW_SHOWNORMAL,
        ewWaitUntilTerminated, ResultCode, @BrowserInstallLog);
      if not Started then
      begin
        Log('Browser installer helper could not be started. ResultCode=' + IntToStr(ResultCode));
        BrowserInstallExitCode := 201;
      end
      else if ResultCode <> 0 then
      begin
        Log('Browser installer helper failed with exit code ' + IntToStr(ResultCode) + '.');
        BrowserInstallExitCode := 200;
      end;
    except
      Log('Browser installer helper execution exception: ' + GetExceptionMessage);
      BrowserInstallExitCode := 202;
    end;
  end;
end;

function GetCustomSetupExitCode: Integer;
begin
  Result := BrowserInstallExitCode;
end;
