$ErrorActionPreference = 'Stop'

$packageName = 'fontlift-win'
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
$url64 = 'https://github.com/fontlaborg/fontlift-win-cli/releases/download/v1.1.24/fontlift-win-v1.1.24.zip'

$packageArgs = @{
  packageName   = $packageName
  unzipLocation = $toolsDir
  url64bit      = $url64
  checksum64    = ''
  checksumType64= 'sha256'
}

Install-ChocolateyZipPackage @packageArgs
