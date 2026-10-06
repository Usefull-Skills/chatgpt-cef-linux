param(
  [string]$VersionOverride = '',
  [string]$PayloadDir = ''
)
$ErrorActionPreference='Stop'
$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Version=if($VersionOverride){$VersionOverride}else{(Get-Content -LiteralPath (Join-Path $Root 'VERSION') -Raw).Trim()}
if($Version -notmatch '^\d+\.\d+\.\d+(?:-[A-Za-z0-9._-]+)?$'){throw "Invalid Browser setup version: $Version"}
if(-not $PayloadDir){$PayloadDir=Join-Path $Root 'build-windows\bin'}
$PayloadDir=[IO.Path]::GetFullPath($PayloadDir)
if(-not(Test-Path -LiteralPath (Join-Path $PayloadDir 'chatgpt-cef-v2.exe') -PathType Leaf)){throw "Browser Windows payload missing: $PayloadDir"}

$candidates=@(
  (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'),
  (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe'),
  (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe')
)
$Iscc=$candidates|Where-Object{$_ -and(Test-Path -LiteralPath $_ -PathType Leaf)}|Select-Object -First 1
if(-not $Iscc){
  $keys=@('HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*','HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*','HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*')
  $entry=Get-ItemProperty $keys -ErrorAction SilentlyContinue|Where-Object{$_.DisplayName -like 'Inno Setup version *'}|Select-Object -First 1
  if($entry.InstallLocation){$candidate=Join-Path ([string]$entry.InstallLocation) 'ISCC.exe';if(Test-Path $candidate){$Iscc=$candidate}}
}
if(-not $Iscc){throw 'Inno Setup 6 compiler is required.'}

$IconOut=Join-Path $Root 'dist\\installer\\remote-commander-browser-setup.ico'
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $IconOut) | Out-Null
Add-Type -AssemblyName System.Drawing
$icon=[System.Drawing.Icon]::ExtractAssociatedIcon((Join-Path $PayloadDir 'chatgpt-cef-v2.exe'))
if(-not $icon){throw 'Could not extract Browser icon from executable.'}
$fs=[IO.File]::Open($IconOut,[IO.FileMode]::Create,[IO.FileAccess]::Write,[IO.FileShare]::None)
try{$icon.Save($fs)}finally{$fs.Dispose();$icon.Dispose()}
if((Get-Item -LiteralPath $IconOut).Length -lt 100){throw 'Extracted Browser setup icon is unexpectedly small.'}

& $Iscc ('/DMyVersion='+$Version) ('/DBrowserPayload='+$PayloadDir) ('/DBrowserSetupIcon='+$IconOut) (Join-Path $PSScriptRoot '..\\installer\\RemoteCommanderBrowser.iss')
if($LASTEXITCODE -ne 0){throw "Browser Inno compile failed: $LASTEXITCODE"}
$Setup=Join-Path $Root ("dist\installer\Remote-Commander-Browser-Setup-v$Version.exe")
if(-not(Test-Path $Setup -PathType Leaf)){throw "Browser setup artifact missing: $Setup"}
$Sha=(Get-FileHash -LiteralPath $Setup -Algorithm SHA256).Hash.ToLowerInvariant()
$Sig=Get-AuthenticodeSignature -LiteralPath $Setup
[ordered]@{
  status='REMOTE_COMMANDER_BROWSER_SETUP_BUILD_PASS'
  version=$Version
  setup=$Setup
  sha256=$Sha
  signatureStatus=[string]$Sig.Status
}|ConvertTo-Json
