# TODO.md
<!-- this_file: TODO.md -->

- [~] Validate automatic uninstall on Windows for both per-user and system installs (see matrix in docs/cleanup-validation.md; run on Win10/Win11).
- [~] Capture console transcripts for the new removal flow and add them to docs (record with tools/collect-fontlift-transcripts.ps1).
- [x] Decide whether a flag is needed to disable auto-uninstall if conflicts appear.
- [x] Build a throwaway registry seeding harness to test `fontlift-win cleanup`.
- [ ] Run cleanup regression on Windows 10 and Windows 11 VMs.
- [x] Draft remediation guidance for FontCache service failures.
- [x] Update the release checklist with cleanup and duplicate-family scenarios.
- [x] Prepare release notes summarizing the new features.
- [ ] Produce troubleshooting screenshots before/after running cleanup.
