param(
  [string]$SourceDir = '',
  [string]$InstallRoot = ''
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Version=(Get-Content -LiteralPath (Join-Path $Root 'VERSION') -Raw).Trim()
if(-not $SourceDir){$SourceDir=Join-Path $Root 'build-windows\bin'}
$SourceDir=[IO.Path]::GetFullPath($SourceDir)
$SourceExe=Join-Path $SourceDir 'chatgpt-cef-v2.exe'
if(-not (Test-Path -LiteralPath $SourceExe -PathType Leaf)){throw "Build Windows before install: $SourceExe"}
if(-not $InstallRoot){$InstallRoot=Join-Path $env:LOCALAPPDATA 'Programs\Remote Commander Browser'}
$InstallRoot=[IO.Path]::GetFullPath($InstallRoot)
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
$CompanionRoot=Join-Path $env:LOCALAPPDATA 'ChatGPTRemoteCommander\browser-companion'

function Ensure-PrivateDirectory([string]$Path) {
  New-Item -ItemType Directory -Force -Path $Path | Out-Null
  $user=[Security.Principal.WindowsIdentity]::GetCurrent().User
  $system=New-Object Security.Principal.SecurityIdentifier('S-1-5-18')
  $admins=New-Object Security.Principal.SecurityIdentifier('S-1-5-32-544')
  $acl=New-Object Security.AccessControl.DirectorySecurity
  $acl.SetOwner($user)
  $acl.SetAccessRuleProtection($true,$false)
  foreach($sid in @($user,$system,$admins)) {
    $rule=New-Object Security.AccessControl.FileSystemAccessRule(
      $sid,[Security.AccessControl.FileSystemRights]::FullControl,
      [Security.AccessControl.InheritanceFlags]'ContainerInherit,ObjectInherit',
      [Security.AccessControl.PropagationFlags]::None,
      [Security.AccessControl.AccessControlType]::Allow)
    [void]$acl.AddAccessRule($rule)
  }
  Set-Acl -LiteralPath $Path -AclObject $acl
}
Ensure-PrivateDirectory $CompanionRoot

New-Item -ItemType Directory -Force -Path $VersionDir,$StartMenu | Out-Null
Copy-Item -Path (Join-Path $SourceDir '*') -Destination $VersionDir -Recurse -Force -ErrorAction Stop
$Logo=Join-Path $Root 'assets\remote-commander-browser-logo.png'
if(Test-Path -LiteralPath $Logo -PathType Leaf){Copy-Item -LiteralPath $Logo -Destination (Join-Path $VersionDir 'remote-commander-browser-logo.png') -Force}

if(Test-Path -LiteralPath $Current){Remove-Item -LiteralPath $Current -Force -Recurse}
New-Item -ItemType Junction -Path $Current -Target $VersionDir | Out-Null

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
Write-Output "REMOTE_COMMANDER_BROWSER_INSTALL_PASS version=$Version package=$($PackageSha.Substring(0,12)) install=$InstallRoot companion=$CompanionRoot shortcut=$Check"
