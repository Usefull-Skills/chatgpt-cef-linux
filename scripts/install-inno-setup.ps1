param(
  [string]$InstallDir = ''
)
$ErrorActionPreference='Stop'
$Version='6.7.3'
$Url='https://github.com/jrsoftware/issrc/releases/download/is-6_7_3/innosetup-6.7.3.exe'
$ExpectedSha256='9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732'
if(-not $InstallDir){$InstallDir=Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6'}
$InstallDir=[IO.Path]::GetFullPath($InstallDir)
$download=Join-Path $env:TEMP ('innosetup-6.7.3-'+[guid]::NewGuid().ToString('N')+'.exe')
try{
  Invoke-WebRequest -UseBasicParsing -Headers @{'User-Agent'='RemoteCommanderBrowser-Release'} -Uri $Url -OutFile $download
  $actual=(Get-FileHash -LiteralPath $download -Algorithm SHA256).Hash.ToLowerInvariant()
  if($actual-ne$ExpectedSha256){throw "INNO_SETUP_SHA256_MISMATCH actual=$actual"}
  $sig=Get-AuthenticodeSignature -LiteralPath $download
  if([string]$sig.Status-ne'Valid'){throw "INNO_SETUP_AUTHENTICODE_INVALID status=$($sig.Status)"}
  $subject=[string]$sig.SignerCertificate.Subject
  if($subject-notlike'*Pyrsys B.V.*'){throw "INNO_SETUP_SIGNER_UNEXPECTED subject=$subject"}
  New-Item -ItemType Directory -Force -Path $InstallDir|Out-Null
  $p=Start-Process -FilePath $download -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/PORTABLE=1',('/DIR="'+$InstallDir+'"')) -Wait -PassThru
  if($p.ExitCode-ne0){throw "INNO_SETUP_INSTALL_FAILED exit=$($p.ExitCode)"}
  $iscc=Join-Path $InstallDir 'ISCC.exe'
  if(-not(Test-Path -LiteralPath $iscc -PathType Leaf)){throw 'INNO_SETUP_ISCC_MISSING'}
  [ordered]@{
    status='INNO_SETUP_PIN_PASS'
    version=$Version
    sha256=$actual
    signer=$subject
    compiler=$iscc
  }|ConvertTo-Json -Compress
}finally{
  Remove-Item -LiteralPath $download -Force -ErrorAction SilentlyContinue
}
