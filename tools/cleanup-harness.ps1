# this_file: tools/cleanup-harness.ps1
# Throwaway harness to seed broken font registry entries and drive cleanup
# made by FontLab https://www.fontlab.com/
param(
    [ValidateSet("User", "System", "Both")]
    [string]$Scope = "User",

    [string]$FontNamePrefix = "FontLiftHarness",

    [int]$EntryCount = 2,

    [string]$BinaryPath = "fontlift-win.exe",

    [switch]$RunCleanup,

    [switch]$VerboseOutput
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Assert-WindowsHost {
    if ($env:OS -notlike "*Windows*") {
        throw "This harness must run on Windows. Aborting to avoid registry edits on non-Windows hosts."
    }
}

function Assert-AdminIfNeeded {
    param([bool]$RequiresAdmin)
    if (-not $RequiresAdmin) { return }
    $currentIdentity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentIdentity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw "System scope requires an elevated shell. Re-run PowerShell as Administrator."
    }
}

function New-OrphanedEntry {
    param(
        [string]$Name,
        [string]$Value,
        [bool]$PerUser
    )

    $root = $PerUser ? "HKCU:" : "HKLM:"
    $path = Join-Path $root "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"

    if ($VerboseOutput) {
        Write-Host "  - Seeding $($PerUser ? "user" : "system") entry:`t$Name -> $Value"
    }

    New-Item -Path $path -Force | Out-Null
    New-ItemProperty -Path $path -Name $Name -Value $Value -PropertyType String -Force | Out-Null
}

function Remove-EntryIfPresent {
    param(
        [string]$Name,
        [bool]$PerUser
    )
    $root = $PerUser ? "HKCU:" : "HKLM:"
    $path = Join-Path $root "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"
    Remove-ItemProperty -Path $path -Name $Name -ErrorAction SilentlyContinue
}

function Get-MissingPathValue {
    param(
        [bool]$PerUser,
        [int]$Index
    )
    if ($PerUser) {
        $userFonts = Join-Path $env:LOCALAPPDATA "Microsoft\\Windows\\Fonts"
        return Join-Path $userFonts ("missing-" + $Index + ".ttf")
    }
    # System registry stores relative filenames
    return ("MissingHarness-" + $Index + ".ttf")
}

function Invoke-FontliftCleanup {
    param([bool]$IncludeSystem)
    $args = @("cleanup")
    if ($IncludeSystem) { $args += "--admin" }

    Write-Host "Running cleanup command: $BinaryPath $($args -join ' ')" -ForegroundColor Cyan
    & $BinaryPath @args
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Cleanup exited with code $LASTEXITCODE. Check console output above."
    } else {
        Write-Host "Cleanup completed with exit code 0." -ForegroundColor Green
    }
}

Assert-WindowsHost
$seedUser = $Scope -in @("User", "Both")
$seedSystem = $Scope -in @("System", "Both")
Assert-AdminIfNeeded -RequiresAdmin:$seedSystem

Write-Host "Seeding $EntryCount orphaned font entries (scope: $Scope)..." -ForegroundColor Cyan
for ($i = 1; $i -le $EntryCount; $i++) {
    $name = "$FontNamePrefix-$i (TrueType)"
    if ($seedUser) {
        Remove-EntryIfPresent -Name $name -PerUser:$true
        $userValue = Get-MissingPathValue -PerUser:$true -Index:$i
        New-OrphanedEntry -Name $name -Value $userValue -PerUser:$true
    }
    if ($seedSystem) {
        Remove-EntryIfPresent -Name $name -PerUser:$false
        $systemValue = Get-MissingPathValue -PerUser:$false -Index:$i
        New-OrphanedEntry -Name $name -Value $systemValue -PerUser:$false
    }
}

Write-Host "Done seeding orphaned entries. Run 'RegEdit' to confirm under Fonts key." -ForegroundColor Green

if ($RunCleanup) {
    Invoke-FontliftCleanup -IncludeSystem:$seedSystem
}

Write-Host "Harness complete. Reminder: Only run on disposable test VMs." -ForegroundColor Yellow
