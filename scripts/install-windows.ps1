param(
  [string]$SourceDir = '',
  [string]$InstallRoot = '',
  [string]$VersionOverride = '',
  [string]$LogoPath = '',
  [string]$CompanionRoot = '',
  [switch]$NoPublicIntegration
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest

$RepoRoot=$null
try{
  $candidate=(Resolve-Path (Join-Path $PSScriptRoot '..') -ErrorAction Stop).Path
  if(Test-Path -LiteralPath (Join-Path $candidate 'VERSION') -PathType Leaf){$RepoRoot=$candidate}
}catch{}

$Version=if($VersionOverride){$VersionOverride}elseif($RepoRoot){(Get-Content -LiteralPath (Join-Path $RepoRoot 'VERSION') -Raw).Trim()}else{throw 'Browser install requires -VersionOverride when repository VERSION is unavailable.'}
if($Version -notmatch '^\d+\.\d+\.\d+(?:-[A-Za-z0-9._-]+)?$'){throw "Invalid Browser version: $Version"}

if(-not $SourceDir){
  if(-not $RepoRoot){throw 'Browser install requires -SourceDir when repository payload is unavailable.'}
  $SourceDir=Join-Path $RepoRoot 'build-windows\bin'
}
$SourceDir=[IO.Path]::GetFullPath($SourceDir)
$SourceExe=Join-Path $SourceDir 'chatgpt-cef-v2.exe'
if(-not (Test-Path -LiteralPath $SourceExe -PathType Leaf)){throw "Browser payload is missing: $SourceExe"}

if(-not $InstallRoot){$InstallRoot=Join-Path $env:LOCALAPPDATA 'Programs\Remote Commander Browser'}
$InstallRoot=[IO.Path]::GetFullPath($InstallRoot)
if(-not $CompanionRoot){$CompanionRoot=Join-Path $env:LOCALAPPDATA 'ChatGPTRemoteCommander\browser-companion'}
$CompanionRoot=[IO.Path]::GetFullPath($CompanionRoot)

if(-not $LogoPath -and $RepoRoot){
  $candidate=Join-Path $RepoRoot 'assets\remote-commander-browser-logo.png'
  if(Test-Path -LiteralPath $candidate -PathType Leaf){$LogoPath=$candidate}
}
if($LogoPath){$LogoPath=[IO.Path]::GetFullPath($LogoPath)}

$Entries = Get-ChildItem -LiteralPath $SourceDir -File -Recurse | ForEach-Object {
  $rel=[IO.Path]::GetRelativePath($SourceDir,$_.FullName).Replace('\','/')
  $h=(Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToLowerInvariant()
  "$rel|$h"
} | Sort-Object
$Manifest=$Entries -join "`n"
$PackageSha=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($Manifest))).ToLowerInvariant()
$VersionDir=Join-Path $InstallRoot (("v"+$Version+"-")+$PackageSha.Substring(0,12))
$Current=Join-Path $InstallRoot 'current'
$StartMenu=Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Remote Commander'
$Reg='HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\RemoteCommanderBrowser'

function Test-PrivateDirectoryAcl([string]$Path) {
  if(-not(Test-Path -LiteralPath $Path -PathType Container)){return $false}
  $user=[Security.Principal.WindowsIdentity]::GetCurrent().User
  $acl=Get-Acl -LiteralPath $Path
  try{$owner=(New-Object Security.Principal.NTAccount($acl.Owner)).Translate([Security.Principal.SecurityIdentifier])}
  catch{return $false}
  if($owner.Value-ne$user.Value -or -not$acl.AreAccessRulesProtected){return $false}

  $expected=@($user.Value,'S-1-5-18','S-1-5-32-544')
  $rules=@($acl.GetAccessRules($true,$false,[Security.Principal.SecurityIdentifier]))
  if($rules.Count-ne$expected.Count){return $false}
  foreach($rule in $rules){
    if([string]$rule.AccessControlType-ne'Allow'){return $false}
    if($expected -notcontains $rule.IdentityReference.Value){return $false}
    if(($rule.FileSystemRights-band[Security.AccessControl.FileSystemRights]::FullControl)-ne[Security.AccessControl.FileSystemRights]::FullControl){return $false}
    $required=[Security.AccessControl.InheritanceFlags]'ContainerInherit,ObjectInherit'
    if(($rule.InheritanceFlags-band$required)-ne$required){return $false}
  }
  return $true
}

function Ensure-PrivateDirectory([string]$Path) {
  New-Item -ItemType Directory -Force -Path $Path | Out-Null
  if(Test-PrivateDirectoryAcl $Path){return}

  $user=[Security.Principal.WindowsIdentity]::GetCurrent().User
  $acl=Get-Acl -LiteralPath $Path
  try{$owner=(New-Object Security.Principal.NTAccount($acl.Owner)).Translate([Security.Principal.SecurityIdentifier])}
  catch{throw 'BROWSER_COMPANION_ACL_OWNER_UNRESOLVED'}
  if($owner.Value-ne$user.Value){throw 'BROWSER_COMPANION_ACL_OWNER_MISMATCH'}

  $icacls=Join-Path $env:SystemRoot 'System32\icacls.exe'
  if(-not(Test-Path -LiteralPath $icacls -PathType Leaf)){throw 'BROWSER_COMPANION_ICACLS_MISSING'}
  $args=@(
    $Path,
    '/inheritance:r',
    '/grant:r',("*"+$user.Value+":(OI)(CI)F"),
    '/grant:r','*S-1-5-18:(OI)(CI)F',
    '/grant:r','*S-1-5-32-544:(OI)(CI)F'
  )
  & $icacls @args | Out-Null
  if($LASTEXITCODE-ne0){throw "BROWSER_COMPANION_ACL_REPAIR_FAILED exit=$LASTEXITCODE"}
  if(-not(Test-PrivateDirectoryAcl $Path)){throw 'BROWSER_COMPANION_ACL_VERIFY_FAILED'}
}
Ensure-PrivateDirectory $CompanionRoot

New-Item -ItemType Directory -Force -Path $VersionDir | Out-Null
Copy-Item -Path (Join-Path $SourceDir '*') -Destination $VersionDir -Recurse -Force -ErrorAction Stop
if($LogoPath -and (Test-Path -LiteralPath $LogoPath -PathType Leaf)){
  Copy-Item -LiteralPath $LogoPath -Destination (Join-Path $VersionDir 'remote-commander-browser-logo.png') -Force
}

if(Test-Path -LiteralPath $Current){Remove-Item -LiteralPath $Current -Force -Recurse}
New-Item -ItemType Junction -Path $Current -Target $VersionDir | Out-Null

$installRecord=[ordered]@{
  schema=1
  product='Remote Commander Browser'
  version=$Version
  packageSha256=$PackageSha
  installedAt=(Get-Date).ToUniversalTime().ToString('o')
  companionRoot=$CompanionRoot
  publicIntegration=(-not $NoPublicIntegration)
}
[IO.File]::WriteAllText((Join-Path $VersionDir 'product-install.json'),(($installRecord|ConvertTo-Json -Depth 5)+[Environment]::NewLine),[Text.UTF8Encoding]::new($false))

if(-not $NoPublicIntegration){
  New-Item -ItemType Directory -Force -Path $StartMenu | Out-Null
  $Wsh=New-Object -ComObject WScript.Shell
  $Shortcut=$Wsh.CreateShortcut((Join-Path $StartMenu 'Remote Commander Browser.lnk'))
  $Shortcut.TargetPath=Join-Path $Current 'chatgpt-cef-v2.exe'
  $Shortcut.WorkingDirectory=$Current
  $Shortcut.IconLocation=(Join-Path $Current 'chatgpt-cef-v2.exe')+',0'
  $Shortcut.Description='Remote Commander Browser'
  $Shortcut.Save()

  $Uninstall=Join-Path $InstallRoot 'uninstall.ps1'
  $UninstallText=@"
`$ErrorActionPreference='Stop'
`$Shortcut=Join-Path `$env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Remote Commander\Remote Commander Browser.lnk'
Remove-Item -LiteralPath `$Shortcut -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\RemoteCommanderBrowser' -Recurse -Force -ErrorAction SilentlyContinue
`$selfRoot='$($InstallRoot.Replace("'","''"))'
Start-Process -FilePath 'cmd.exe' -ArgumentList '/d','/c',"timeout /t 1 /nobreak >nul & rmdir /s /q `"`$selfRoot`"" -WindowStyle Hidden
"@
  [IO.File]::WriteAllText($Uninstall,$UninstallText,(New-Object Text.UTF8Encoding($false)))

  New-Item -Path $Reg -Force | Out-Null
  New-ItemProperty -Path $Reg -Name DisplayName -Value 'Remote Commander Browser' -PropertyType String -Force | Out-Null
  New-ItemProperty -Path $Reg -Name DisplayVersion -Value $Version -PropertyType String -Force | Out-Null
  New-ItemProperty -Path $Reg -Name Publisher -Value 'Remote Commander' -PropertyType String -Force | Out-Null
  New-ItemProperty -Path $Reg -Name InstallLocation -Value $InstallRoot -PropertyType String -Force | Out-Null
  New-ItemProperty -Path $Reg -Name DisplayIcon -Value ((Join-Path $Current 'chatgpt-cef-v2.exe')+',0') -PropertyType String -Force | Out-Null
  New-ItemProperty -Path $Reg -Name UninstallString -Value ('"'+(Get-Command pwsh.exe).Source+'" -NoProfile -File "'+$Uninstall+'"') -PropertyType String -Force | Out-Null
  New-ItemProperty -Path $Reg -Name NoModify -Value 1 -PropertyType DWord -Force | Out-Null
  New-ItemProperty -Path $Reg -Name NoRepair -Value 1 -PropertyType DWord -Force | Out-Null

  $Check=Join-Path $StartMenu 'Remote Commander Browser.lnk'
  if(-not (Test-Path -LiteralPath $Check -PathType Leaf)){throw 'Browser Start Menu shortcut was not created'}
}

Write-Output "REMOTE_COMMANDER_BROWSER_INSTALL_PASS version=$Version package=$($PackageSha.Substring(0,12)) install=$InstallRoot companion=$CompanionRoot public=$(-not $NoPublicIntegration)"
