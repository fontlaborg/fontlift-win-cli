# PLAN.md
<!-- this_file: PLAN.md -->

## Project Overview

**fontlift-win-cli** – Windows CLI tool for font installation, removal, listing, and system cleanup.

**One-sentence scope:** Install, uninstall, list, remove, and clean fonts on Windows without external dependencies.

## Objective 1: Validate Automatic Uninstall Flow
- Assemble a Windows test matrix (per-user vs system installs) to verify that legacy fonts are removed before new copies are registered.
- Capture console transcripts for success, permission-denied, and invalid-path scenarios to embed in documentation.
- Decide whether to expose a suppress-auto-uninstall flag after validation (only if conflicts are reported).

## Objective 2: Harden Cleanup Command
- Build a throwaway test harness that seeds orphaned registry entries and confirms `fontlift cleanup` removes them.
- Exercise cache purging on Windows 10 and Windows 11 to ensure the FontCache service stop/start loop behaves consistently.
- Document remediation guidance when the service cannot be stopped (e.g., antivirus interference) and evaluate adding retries.

## Objective 3: Release Packaging Follow-up
- Update the release checklist to include a `fontlift cleanup` smoke test and duplicate-family install scenario.
- Prepare the next release notes highlighting the new cleanup command and automatic uninstall behavior.
- Produce troubleshooting examples (before/after screenshots) illustrating the impact of cache purging.

## Dependencies & Risks
- Requires administrator privileges and a disposable Windows environment for end-to-end validation.
- No automated CI exists yet; manual verification remains mandatory until the test harness is delivered.
- Registry modifications carry inherent risk—tests must never run against production machines.
