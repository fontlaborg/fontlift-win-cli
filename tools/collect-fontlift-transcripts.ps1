# this_file: tools/collect-fontlift-transcripts.ps1
# Wrapper to record real fontlift console transcripts for docs/support
# made by FontLab https://www.fontlab.com/
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("cleanup-user", "cleanup-admin", "uninstall-missing", "auto-uninstall")]
    [string]$Scenario,

    [string]$BinaryPath = "fontlift-win.exe",

    [string]$OutputDir = "transcripts",

    [string]$FontPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Assert-WindowsHost {
    if ($env:OS -notlike "*Windows*") {
        throw "This recorder must run on Windows. Aborting to avoid invalid results."
    }
}

function Assert-AdminIfNeeded {
    param([bool]$RequiresAdmin)
    if (-not $RequiresAdmin) { return }
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw "Scenario '$Scenario' expects an elevated PowerShell session (Run as Administrator)."
    }
}

function Run-Command {
    param([string[]]$Args)
    $display = $Args -join " "
    Write-Host "Running: $BinaryPath $display" -ForegroundColor Cyan
    & $BinaryPath @Args | Out-Default
    $exit = $LASTEXITCODE
    Write-Host "ExitCode: $exit" -ForegroundColor Yellow
}

Assert-WindowsHost

if ($Scenario -eq "cleanup-admin") {
    Assert-AdminIfNeeded -RequiresAdmin:$true
}

if ($Scenario -eq "auto-uninstall" -and [string]::IsNullOrWhiteSpace($FontPath)) {
    throw "auto-uninstall scenario requires -FontPath pointing to a valid font file."
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$transcriptPath = Join-Path $OutputDir ("fontlift-" + $Scenario + "-" + $timestamp + ".txt")

Start-Transcript -Path $transcriptPath | Out-Null

switch ($Scenario) {
    "cleanup-user" {
        Run-Command @("cleanup")
    }
    "cleanup-admin" {
        Run-Command @("cleanup", "--admin")
    }
    "uninstall-missing" {
        Run-Command @("uninstall", "-n", "MissingFont")
    }
    "auto-uninstall" {
        Run-Command @("install", $FontPath)
        Run-Command @("install", $FontPath)
    }
    default {
        throw "Unhandled scenario: $Scenario"
    }
}

Stop-Transcript | Out-Null
Write-Host "Transcript saved to $transcriptPath" -ForegroundColor Green

