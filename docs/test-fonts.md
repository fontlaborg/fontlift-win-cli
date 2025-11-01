---
this_file: docs/test-fonts.md
---

# Test Fonts

## Purpose
Automated and manual verification rely on known-good font files. This guide lists freely distributable fonts that exercise the code paths for `.ttf`, `.otf`, and `.ttc`/`.otc` handling without creating licensing friction.

## Recommended Set
| Format | Font | Source | Notes |
|--------|------|--------|-------|
| `.ttf` | Noto Sans Regular | https://fonts.google.com/specimen/Noto+Sans | OFL-licensed, includes standard Latin tables. |
| `.otf` | Fira Code Regular | https://github.com/tonsky/FiraCode/releases | SIL Open Font License; contains OpenType features. |
| `.ttc` | Noto Sans CJK JP | https://www.google.com/get/noto/#sans-jpan | Large collection; stresses collection parsing. |
| `.otc` | Source Han Serif | https://github.com/adobe-fonts/source-han-serif/releases | Verify OTC handling; download the `*.otc` asset. |

All listed fonts permit redistribution for testing under the SIL Open Font License (OFL). Always retain upstream LICENSE files alongside downloads.

## Storage Layout
- Place downloaded files under `tests/test_fonts/` (create the directory if needed).
- Preserve original filenames to match upstream documentation.
- Commit hashes or release tags should be noted in `WORK.md` when updating assets.

## Usage Checklist
- Confirm SHA256 checksums before committing fonts to avoid corruption.
- Install each font manually with `fontlift install` once to validate Windows integration.
- Keep font archives outside the repository; store only extracted `.ttf/.otf/.ttc/.otc` files required for tests.
- When adding new fonts, update this document and reference the change in `CHANGELOG.md`.

## License Reminder
Respect the OFL requirements: credit the original authors, do not sell the fonts by themselves, and include the OFL license text with any redistribution or packaging.
