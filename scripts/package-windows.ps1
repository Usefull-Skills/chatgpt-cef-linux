param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Version=(Get-Content -LiteralPath (Join-Path $Root 'VERSION') -Raw).Trim()
$Bin=Join-Path $Root 'build-windows\bin'
$Dist=Join-Path $Root 'dist'
if(!(Test-Path -LiteralPath (Join-Path $Bin 'chatgpt-cef-v2.exe') -PathType Leaf)){ throw 'Build Windows before packaging.' }
New-Item -ItemType Directory -Force $Dist | Out-Null
$Zip=Join-Path $Dist ("remote-commander-browser-v$Version-windows-x86_64.zip")
Remove-Item -LiteralPath $Zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $Bin '*') -DestinationPath $Zip -CompressionLevel Optimal
$Sha=(Get-FileHash -Algorithm SHA256 -LiteralPath $Zip).Hash.ToLowerInvariant()
$Sums=Join-Path $Dist ("SHA256SUMS-windows-v$Version.txt")
"$Sha  $([IO.Path]::GetFileName($Zip))" | Set-Content -LiteralPath $Sums -Encoding ascii
Write-Output "WINDOWS_PACKAGE_PASS product=RemoteCommanderBrowser zip=$Zip sha256=$Sha"
