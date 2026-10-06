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

if(Test-Path -LiteralPath $CefRoot){
  $readme=Join-Path $CefRoot 'README.txt'
  if((Test-Path $readme) -and (Select-String -LiteralPath $readme -SimpleMatch 'CEF Version:      154.0.32+g682c378+chromium-154.0.8037.58' -Quiet)){
    Write-Output "CEF_FETCH_CACHE_HIT root=$CefRoot"
    Write-Output "CEF_ARCHIVE_SHA256=$ExpectedSha256"
    exit 0
  }
  throw 'Existing .deps/cef-windows does not match the pinned version; refusing implicit replacement.'
}

if(Test-Path -LiteralPath $Archive){
  Write-Output "CEF_FETCH_STAGE verify-existing started=$([DateTime]::UtcNow.ToString('o'))"
  Assert-Archive $Archive
}else{
  $part=$Archive+'.part'
  Remove-Item -LiteralPath $part -Force -ErrorAction SilentlyContinue
  Write-Output "CEF_FETCH_STAGE download started=$([DateTime]::UtcNow.ToString('o'))"
  & curl.exe --location --fail --retry 4 --retry-delay 2 --connect-timeout 20 --max-time 600 --output $part $Url
  if($LASTEXITCODE -ne 0){ throw "CEF download failed exit=$LASTEXITCODE" }
  Write-Output "CEF_FETCH_STAGE verify-download started=$([DateTime]::UtcNow.ToString('o'))"
  Assert-Archive $part
  Move-Item -LiteralPath $part -Destination $Archive
}

$SevenZip=(Get-Command 7z.exe -ErrorAction SilentlyContinue)
$Tar=(Get-Command tar.exe -ErrorAction SilentlyContinue)
if(-not $SevenZip -and -not $Tar){ throw 'CEF extraction requires either Windows tar.exe or 7z.exe.' }
$tmp=Join-Path $Deps ('.cef-win-extract.'+[Guid]::NewGuid().ToString('N'))
$payload=Join-Path $tmp 'payload'
New-Item -ItemType Directory -Force $tmp,$payload | Out-Null
try{
  if($SevenZip){
    Write-Output "CEF_FETCH_STAGE extract-bzip2-7z started=$([DateTime]::UtcNow.ToString('o'))"
    & $SevenZip.Source x -y "-o$tmp" $Archive
    if($LASTEXITCODE -ne 0){ throw "7z bzip2 extraction failed exit=$LASTEXITCODE" }
    $intermediate=Get-ChildItem -LiteralPath $tmp -Filter '*.tar' -File | Select-Object -First 1
    if(!$intermediate){ throw 'Intermediate CEF tar missing after bzip2 extraction' }
    Write-Output "CEF_FETCH_STAGE extract-tar-7z started=$([DateTime]::UtcNow.ToString('o'))"
    & $SevenZip.Source x -y "-o$payload" $intermediate.FullName
    if($LASTEXITCODE -ne 0){ throw "7z tar extraction failed exit=$LASTEXITCODE" }
  }else{
    Write-Output "CEF_FETCH_STAGE extract-bsdtar started=$([DateTime]::UtcNow.ToString('o'))"
    & $Tar.Source -xjf $Archive -C $payload
    if($LASTEXITCODE -ne 0){ throw "Windows tar extraction failed exit=$LASTEXITCODE" }
  }
  $extracted=Join-Path $payload $Name
  if(!(Test-Path -LiteralPath $extracted -PathType Container)){ throw 'Unexpected CEF archive layout' }
  Move-Item -LiteralPath $extracted -Destination $CefRoot
}finally{
  Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue
}
$readme=Join-Path $CefRoot 'README.txt'
if(!(Test-Path $readme) -or !(Select-String -LiteralPath $readme -SimpleMatch 'CEF Version:      154.0.32+g682c378+chromium-154.0.8037.58' -Quiet)){ throw 'Extracted CEF version marker mismatch' }
Write-Output "CEF_FETCH_PASS root=$CefRoot completed=$([DateTime]::UtcNow.ToString('o'))"
Write-Output "CEF_ARCHIVE_SHA256=$ExpectedSha256"
