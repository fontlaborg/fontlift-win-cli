# this_file: scripts/get-version.ps1

param(
    [string]$TargetVersion,
    [ValidateSet('String', 'Json', 'Object')]
    [string]$Format = 'String'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Convert-ToResultObject {
    param(
        [string]$Base,
        [string]$SemVer,
        [string]$Tag
    )

    switch ($Format) {
        'Json' { return [pscustomobject]@{ base = $Base; semver = $SemVer; tag = $Tag } | ConvertTo-Json -Compress }
        'Object' { return [pscustomobject]@{ base = $Base; semver = $SemVer; tag = $Tag } }
        default {
            Write-Output ("BASE={0}" -f $Base)
            Write-Output ("SEMVER={0}" -f $SemVer)
            Write-Output ("TAG={0}" -f $Tag)
        }
    }
}

function Resolve-FromProvidedVersion {
    param([string]$Input)

    $clean = $Input.Trim()
    if (-not $clean) {
        throw [ArgumentException]::new("Version string cannot be empty.")
    }

    $match = [regex]::Match($clean, '^(?<base>\d+\.\d+\.\d+)(?<suffix>[-+].*)?$')
    if (-not $match.Success) {
        throw [ArgumentException]::new("Invalid semantic version: $Input")
    }

    $base = $match.Groups['base'].Value
    $semVer = if ($match.Groups['suffix'].Value) { $clean } else { $base }
    $tag = "v$semVer"

    return [pscustomobject]@{
        Base   = $base
        SemVer = $semVer
        Tag    = $tag
    }
}

function Resolve-FromGit {
    # Attempt to resolve version from git tags
    # Returns $null if git is not available, no tags exist, or describe fails
    $describe = git describe --tags --match 'v[0-9]*.[0-9]*.[0-9]*' --long --dirty 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $describe) {
        return $null
    }

    $pattern = '^v(?<base>\d+\.\d+\.\d+)-(?<distance>\d+)-g(?<sha>[0-9a-f]+)(?<dirty>-dirty)?$'
    $parsed = [regex]::Match($describe.Trim(), $pattern)
    if (-not $parsed.Success) {
        throw "Unable to parse git describe output: $describe"
    }

    $base = $parsed.Groups['base'].Value
    $distance = [int]$parsed.Groups['distance'].Value
    $sha = $parsed.Groups['sha'].Value
    $dirty = $parsed.Groups['dirty'].Success

    if ($distance -eq 0 -and -not $dirty) {
        return [pscustomobject]@{
            Base   = $base
            SemVer = $base
            Tag    = "v$base"
        }
    }

    if (-not $sha) {
        $sha = git rev-parse --short HEAD
    }

    $preParts = @("dev", $distance)
    if ($dirty) {
        $preParts += "dirty"
    }
    $preRelease = ($preParts -join '.')

    $semVer = "$base-$preRelease"
    if ($sha) {
        $semVer = "$semVer+g$sha"
    }

    return [pscustomobject]@{
        Base   = $base
        SemVer = $semVer
        Tag    = "v$semVer"
    }
}

function Resolve-Fallback {
    # Fallback version for repos without tags or when git is unavailable
    # Returns 0.0.0-dev.{count}+g{sha} format
    # Handles missing git gracefully with "unknown" and count 0
    $sha = git rev-parse --short HEAD 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $sha) {
        $sha = "unknown"
    }

    $count = git rev-list --count HEAD 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $count) {
        $count = 0
    }

    $base = "0.0.0"
    $semVer = "{0}-dev.{1}+g{2}" -f $base, $count, $sha

    return [pscustomobject]@{
        Base   = $base
        SemVer = $semVer
        Tag    = "v$semVer"
    }
}

$result =
    if ($TargetVersion) {
        Resolve-FromProvidedVersion -Input $TargetVersion
    } else {
        $gitResult = Resolve-FromGit
        if ($gitResult) {
            $gitResult
        } else {
            Resolve-Fallback
        }
    }

Convert-ToResultObject -Base $result.Base -SemVer $result.SemVer -Tag $result.Tag
