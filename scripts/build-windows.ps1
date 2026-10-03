param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest

$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Write-Output "WINDOWS_BUILD_STAGE fetch started=$([DateTime]::UtcNow.ToString('o'))"
& (Join-Path $PSScriptRoot 'fetch-cef-windows.ps1')
if($LASTEXITCODE -ne 0){ throw "CEF fetch failed exit=$LASTEXITCODE" }

$CefRoot=Join-Path $Root '.deps\cef-windows'
$Build=Join-Path $Root 'build-windows'
$VsWhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if(!(Test-Path -LiteralPath $VsWhere)){ throw "vswhere missing: $VsWhere" }
$Vs=(& $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
if([string]::IsNullOrWhiteSpace($Vs)){ throw 'Visual Studio C++ toolchain not found' }
$DevCmd=Join-Path $Vs 'Common7\Tools\VsDevCmd.bat'
if(!(Test-Path -LiteralPath $DevCmd)){ throw "VsDevCmd missing: $DevCmd" }
$Ninja=(Get-Command ninja.exe -ErrorAction Stop).Source
$CMake=(Get-Command cmake.exe -ErrorAction Stop).Source
$Parallel=[Math]::Max(2,[Math]::Min(8,[Environment]::ProcessorCount))
Remove-Item -LiteralPath $Build -Recurse -Force -ErrorAction SilentlyContinue

Write-Output "WINDOWS_BUILD_STAGE configure-generator=Ninja parallel=$Parallel started=$([DateTime]::UtcNow.ToString('o'))"
$inner='call "'+$DevCmd+'" -no_logo -arch=x64 -host_arch=x64 && "'+$CMake+'" -S "'+$Root+'" -B "'+$Build+'" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCEF_ROOT="'+$CefRoot+'" && "'+$CMake+'" --build "'+$Build+'" --parallel '+$Parallel+' --target chatgpt-cef-v2'
& cmd.exe /d /s /c ('"'+$inner+'"')
if($LASTEXITCODE -ne 0){ throw "Ninja configure/build failed exit=$LASTEXITCODE" }

$Exe=Join-Path $Build 'bin\chatgpt-cef-v2.exe'
if(!(Test-Path -LiteralPath $Exe -PathType Leaf)){ throw "Windows executable missing: $Exe" }
Write-Output "WINDOWS_BUILD_PASS exe=$Exe completed=$([DateTime]::UtcNow.ToString('o'))"
