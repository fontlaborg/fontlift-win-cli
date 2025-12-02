# cleanup-validation.md
<!-- this_file: docs/cleanup-validation.md -->
made by FontLab https://www.fontlab.com/

## Purpose
Provide a repeatable, disposable workflow to validate automatic uninstall behavior and the `cleanup` command on Windows VMs without touching production machines.

## Test Matrix (Windows 10/11)
Scope: run on Windows 10 22H2 and Windows 11 24H2 (English locale OK). Capture evidence with `tools/collect-fontlift-transcripts.ps1` into `transcripts/`.

| Scenario | Scope | Steps | Evidence | Status |
| --- | --- | --- | --- | --- |
| Auto-uninstall on duplicate install | User | `install <font>` then `install <same font>`; expect first copy removed automatically, exit 0 | Transcript `auto-uninstall`, screenshot of Fonts registry (HKCU) showing single entry | Pending |
| Auto-uninstall on duplicate install | System (admin) | Same as above from elevated shell | Transcript `auto-uninstall --admin if needed`, registry (HKLM) single entry | Pending |
| Cleanup removes seeded harness entries | User | Run `tools/cleanup-harness.ps1 -Scope User -EntryCount 2 -RunCleanup -VerboseOutput` | Transcript `cleanup-user`, before/after reg query | Pending |
| Cleanup removes seeded harness entries | System (admin) | `tools/cleanup-harness.ps1 -Scope Both -EntryCount 2 -RunCleanup -VerboseOutput` from elevated PowerShell | Transcript `cleanup-admin`, before/after reg query | Pending |
| Font cache purge loop | System (admin) | Run `fontlift-win cleanup --admin`; confirm FontCache service stop/start succeeds and caches removed | Transcript `cleanup-admin`, screenshot of cache directories before/after | Pending |
| Permission-denied guardrail | System (non-admin) | Run `fontlift-win cleanup --admin` from non-elevated shell | Transcript showing exit code 2 + guidance | Pending |

## 1) Orphaned Registry Harness (throwaway only)
- Script: `tools/cleanup-harness.ps1`
- What it does: seeds fake registry values that point at missing font files, then optionally runs `fontlift-win cleanup`.
- Usage:
  ```powershell
  # User scope only (no admin)
  pwsh -f tools/cleanup-harness.ps1 -Scope User -EntryCount 2 -RunCleanup -VerboseOutput

  # System + user scopes (requires elevated PowerShell)
  pwsh -f tools/cleanup-harness.ps1 -Scope Both -EntryCount 3 -RunCleanup -VerboseOutput
  ```
- Safety rails:
  - Aborts on non-Windows hosts.
  - Requires elevation for system scope.
- Writes obvious placeholder values (`MissingHarness-*.ttf`) so cleanup should remove them all.
- Run only on disposable VMs.

## 2) Console Transcripts (representative)
Use `tools/collect-fontlift-transcripts.ps1` to capture real output (it pipes native output through `Out-Default` so `Start-Transcript` records exe output).citeturn0search1

Examples:
- User cleanup: `pwsh -f tools/collect-fontlift-transcripts.ps1 -Scenario cleanup-user`
- Permission-denied check: run the same script with `-Scenario cleanup-admin` from a **non-admin** shell (expected exit code 2).
- Auto-uninstall proof: `pwsh -f tools/collect-fontlift-transcripts.ps1 -Scenario auto-uninstall -FontPath C:\path\to\font.ttf`

Use the harness to seed entries, then run these commands to capture transcripts for docs/support:

### A. Success — user scope cleanup
```
> fontlift-win cleanup
Starting user cleanup...
Scanning font registry for broken entries...
  - Removing broken entry: FontLiftHarness-1 (TrueType)
    File not found: C:\Users\Test\AppData\Local\Microsoft\Windows\Fonts\missing-1.ttf
  - Removing broken entry: FontLiftHarness-2 (TrueType)
    File not found: C:\Users\Test\AppData\Local\Microsoft\Windows\Fonts\missing-2.ttf
Found and removed 2 broken font entries.
Clearing font caches...
  - Clearing Windows user font cache directories...
  - Removing Adobe cache files (LocalAppData)...
  - Removing Adobe cache files (AppData)...
Font caches cleared successfully.
User cleanup completed successfully.
```

### B. Permission denied — system cleanup without elevation
```
> fontlift-win cleanup --admin
Starting system cleanup...
Error: Administrator privileges are required for --admin cleanup.
Solution: Right-click Command Prompt and select 'Run as administrator'.
```

### C. Invalid path during uninstall by name (pre-install guard)
```
> fontlift-win uninstall -n "MissingFont"
Error: Font not found in registry: MissingFont
```

## 3) Remediation for FontCache service failures
- If `cleanup --admin` reports it cannot stop `FontCache`:
  - Stop **Windows Font Cache Service**, delete `C:\Windows\ServiceProfiles\LocalService\AppData\Local\FontCache\*` and `C:\Windows\System32\FNTCACHE.DAT`, then start the service again.citeturn0search2
  - If files stay locked, reboot once with the service disabled, delete the cache files, then re-enable/start the service and rerun `fontlift-win cleanup --admin`.citeturn0search9

## 4) Release checklist additions
- Add to release steps:
  - Run `fontlift-win install <font>` twice and confirm the second run auto-uninstalls the first copy (no duplicate registry entries remain).
  - Seed orphaned registry entries with `tools/cleanup-harness.ps1 -Scope Both -EntryCount 2 -RunCleanup` on a Windows 10 VM and on a Windows 11 VM; expect exit code 0 and zero remaining harness entries.
  - Capture a fresh transcript of `cleanup` success (user scope) and permission-denied (`--admin` without elevation) for the release notes.

## 5) Decision on suppressing auto-uninstall flag
- Current behavior removes any existing fonts that share the family name before installation.
- No conflicts have been reported; keeping auto-uninstall as the default reduces duplicate-family failures and simplifies support.
- Decision: **No suppress flag for now.** Revisit only if users supply conflicting workflow requirements.

## 6) Screenshot recipe (before/after)
- Before running cleanup, capture Explorer screenshots of:
  - `%LOCALAPPDATA%\Microsoft\Windows\Fonts` (user scope)
  - `C:\Windows\ServiceProfiles\LocalService\AppData\Local\FontCache` and `C:\Windows\System32\FNTCACHE.DAT` for system scope.citeturn0search2
- After cleanup, grab the same views to show removed cache files and shrinking directory sizes.
- Note whether the **Windows Font Cache Service** is running (services.msc) after cleanup; default startup type is Manual/Trigger Start on Win10/11.citeturn0search9

## 7) Next steps (still pending)
- Execute the matrix above on Windows 10 and Windows 11 VMs and replace the representative transcripts with recorded ones from `transcripts/`.
- Capture the before/after cache screenshots per recipe and embed them here.
