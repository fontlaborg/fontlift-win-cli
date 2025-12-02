#!/usr/bin/env bash
# this_file: tools/test-binary-name.sh
# Verifies the project is configured to build and package fontlift-win.exe
# Fails fast if any key files still reference the old binary name.

set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

assert_contains() {
  local file="$1"
  local pattern="$2"
  if ! grep -Fq "$pattern" "$file"; then
    echo "Expected pattern not found: '$pattern' in $file" >&2
    exit 1
  fi
}

assert_contains "$root_dir/build.cmd" "/OUT:build\\fontlift-win.exe"
assert_contains "$root_dir/publish.cmd" "build\\fontlift-win.exe"
assert_contains "$root_dir/publish.cmd" "fontlift-win-"
assert_contains "$root_dir/templates/version.rc.template" "\"InternalName\", \"fontlift-win\""
assert_contains "$root_dir/templates/version.rc.template" "\"OriginalFilename\", \"fontlift-win.exe\""
assert_contains "$root_dir/src/main.cpp" "fontlift-win - Windows Font Management CLI"
assert_contains "$root_dir/README.md" "fontlift-win list"

echo "Binary name checks passed."
