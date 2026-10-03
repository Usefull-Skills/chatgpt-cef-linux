param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
& (Join-Path $PSScriptRoot 'fetch-cef-windows.ps1')
if($LASTEXITCODE -ne 0){ throw "CEF fetch failed exit=$LASTEXITCODE" }

$CefRoot=Join-Path $Root '.deps\cef-windows'
$Build=Join-Path $Root 'build-windows'
& cmake.exe -S $Root -B $Build -G 'Visual Studio 17 2022' -A x64 "-DCEF_ROOT=$CefRoot"
if($LASTEXITCODE -ne 0){ throw "CMake configure failed exit=$LASTEXITCODE" }
& cmake.exe --build $Build --config Release --target chatgpt-cef-v2 -- /m
if($LASTEXITCODE -ne 0){ throw "CMake build failed exit=$LASTEXITCODE" }

$Exe=Join-Path $Build 'bin\chatgpt-cef-v2.exe'
if(!(Test-Path -LiteralPath $Exe -PathType Leaf)){ throw "Windows executable missing: $Exe" }
Write-Output "WINDOWS_BUILD_PASS exe=$Exe"
