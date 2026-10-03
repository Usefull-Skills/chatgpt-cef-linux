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

$Parallel=[Math]::Max(2,[Math]::Min(8,[Environment]::ProcessorCount))
Remove-Item -LiteralPath $Build -Recurse -Force -ErrorAction SilentlyContinue

Write-Output "WINDOWS_BUILD_STAGE import-msvc-env started=$([DateTime]::UtcNow.ToString('o'))"
$TempRoot=if($env:RUNNER_TEMP){$env:RUNNER_TEMP}else{[IO.Path]::GetTempPath()}
$EnvCmd=Join-Path $TempRoot ("cgwa-vsenv-"+[Guid]::NewGuid().ToString('N')+".cmd")
try{
  @(
    '@echo off'
    ('call "'+$DevCmd+'" -no_logo -arch=x64 -host_arch=x64 >nul')
    'if errorlevel 1 exit /b %errorlevel%'
    'set'
  ) | Set-Content -LiteralPath $EnvCmd -Encoding Ascii

  $envLines=@(& cmd.exe /d /c $EnvCmd)
  if($LASTEXITCODE -ne 0){ throw "VsDevCmd environment import failed exit=$LASTEXITCODE" }
} finally {
  Remove-Item -LiteralPath $EnvCmd -Force -ErrorAction SilentlyContinue
}

foreach($line in $envLines){
  $eq=$line.IndexOf('=')
  if($eq -le 0){ continue }
  $name=$line.Substring(0,$eq)
  $value=$line.Substring($eq+1)
  if($name -match '^[A-Za-z_][A-Za-z0-9_()]*$'){
    Set-Item -LiteralPath ("Env:"+$name) -Value $value
  }
}

$Ninja=(Get-Command ninja.exe -ErrorAction Stop).Source
$CMake=(Get-Command cmake.exe -ErrorAction Stop).Source
Write-Output "WINDOWS_BUILD_STAGE tools cmake=$CMake ninja=$Ninja"

Write-Output "WINDOWS_BUILD_STAGE configure-generator=Ninja parallel=$Parallel started=$([DateTime]::UtcNow.ToString('o'))"
& $CMake -S $Root -B $Build -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCEF_ROOT=$CefRoot"
if($LASTEXITCODE -ne 0){ throw "Ninja configure failed exit=$LASTEXITCODE" }

Write-Output "WINDOWS_BUILD_STAGE compile parallel=$Parallel started=$([DateTime]::UtcNow.ToString('o'))"
& $CMake --build $Build --parallel $Parallel --target chatgpt-cef-v2
if($LASTEXITCODE -ne 0){ throw "Ninja build failed exit=$LASTEXITCODE" }

$Exe=Join-Path $Build 'bin\chatgpt-cef-v2.exe'
if(!(Test-Path -LiteralPath $Exe -PathType Leaf)){ throw "Windows executable missing: $Exe" }
Write-Output "WINDOWS_BUILD_PASS exe=$Exe completed=$([DateTime]::UtcNow.ToString('o'))"
