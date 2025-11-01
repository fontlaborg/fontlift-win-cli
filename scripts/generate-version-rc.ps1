# this_file: scripts/generate-version-rc.ps1

param(
    [Parameter(Mandatory = $true)]
    [string]$TargetVersion
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$clean = $TargetVersion.Trim()
if (-not $clean) {
    throw [ArgumentException]::new("Version string cannot be empty.")
}

$pattern = '^(?<major>\d+)\.(?<minor>\d+)\.(?<patch>\d+)(?<suffix>[-+].*)?$'
$match = [regex]::Match($clean, $pattern)
if (-not $match.Success) {
    throw [ArgumentException]::new("Invalid semantic version for version.rc: $TargetVersion")
}

$major = [int]$match.Groups['major'].Value
$minor = [int]$match.Groups['minor'].Value
$patch = [int]$match.Groups['patch'].Value
$baseVersion = "{0}.{1}.{2}" -f $major, $minor, $patch
$displayVersion = if ($match.Groups['suffix'].Value) { $clean } else { $baseVersion }

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$templatePath = Join-Path $scriptDir '..\templates\version.rc.template'
$outputPath = Join-Path $scriptDir '..\src\version.rc'

if (-not (Test-Path -Path $templatePath)) {
    throw "Template not found: $templatePath"
}

$content = Get-Content -Path $templatePath -Raw
$content = $content -replace '@VERSION_MAJOR@', $major
$content = $content -replace '@VERSION_MINOR@', $minor
$content = $content -replace '@VERSION_PATCH@', $patch
$content = $content -replace '@VERSION_STRING@', $displayVersion

Set-Content -Path $outputPath -Value $content -Encoding Ascii
Write-Host ("Generated {0} using version {1}" -f $outputPath, $displayVersion)
