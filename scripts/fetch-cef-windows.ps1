param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest

$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Deps=Join-Path $Root '.deps'
$Downloads=Join-Path $Deps 'downloads'
$CefRoot=Join-Path $Deps 'cef-windows'
$Name='cef_binary_154.0.32+g682c378+chromium-154.0.8037.58_windows64_minimal'
$Archive=Join-Path $Downloads ($Name+'.tar.bz2')
$Url='https://cef-builds.spotifycdn.com/cef_binary_154.0.32%2Bg682c378%2Bchromium-154.0.8037.58_windows64_minimal.tar.bz2'
$ExpectedSha256='aa1f7ab28005307edcc13f95e3ce5cd9d221894b00458fb5e368c00603204a2a'
$ExpectedBytes=172827308

New-Item -ItemType Directory -Force $Downloads | Out-Null

function Assert-Archive {
  param([string]$Path)
  if(!(Test-Path -LiteralPath $Path -PathType Leaf)){ throw 'CEF archive missing' }
  $item=Get-Item -LiteralPath $Path
  if($item.Length -ne $ExpectedBytes){ throw "CEF archive size mismatch expected=$ExpectedBytes actual=$($item.Length)" }
  $sha=(Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
  if($sha -ne $ExpectedSha256){ throw "CEF SHA256 mismatch expected=$ExpectedSha256 actual=$sha" }
}

if(Test-Path -LiteralPath $Archive){
  Assert-Archive $Archive
}else{
  $part=$Archive+'.part'
  Remove-Item -LiteralPath $part -Force -ErrorAction SilentlyContinue
  Invoke-WebRequest -Uri $Url -OutFile $part
  Assert-Archive $part
  Move-Item -LiteralPath $part -Destination $Archive
}

if(Test-Path -LiteralPath $CefRoot){
  $readme=Join-Path $CefRoot 'README.txt'
  if((Test-Path $readme) -and (Select-String -LiteralPath $readme -SimpleMatch 'CEF Version:      154.0.32+g682c378+chromium-154.0.8037.58' -Quiet)){
    Write-Output "CEF_ROOT=$CefRoot"
    Write-Output "CEF_ARCHIVE_SHA256=$ExpectedSha256"
    exit 0
  }
  throw 'Existing .deps/cef-windows does not match the pinned version; refusing implicit replacement.'
}

$tmp=Join-Path $Deps ('.cef-win-extract.'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $tmp | Out-Null
try{
  & tar.exe -xjf $Archive -C $tmp
  if($LASTEXITCODE -ne 0){ throw "tar extraction failed exit=$LASTEXITCODE" }
  $extracted=Join-Path $tmp $Name
  if(!(Test-Path -LiteralPath $extracted -PathType Container)){ throw 'Unexpected CEF archive layout' }
  Move-Item -LiteralPath $extracted -Destination $CefRoot
}finally{
  Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Output "CEF_ROOT=$CefRoot"
Write-Output "CEF_ARCHIVE_SHA256=$ExpectedSha256"
