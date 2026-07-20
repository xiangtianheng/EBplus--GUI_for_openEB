#!/usr/bin/env bash
# Guards the macOS CI against Homebrew's unversioned OpenCV formula changing.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
workflow="$repo_root/.github/workflows/macos.yml"

require_count() {
    local needle="$1"
    local expected="$2"
    local actual
    actual="$(rg -F -c -- "$needle" "$workflow" || true)"
    actual="${actual:-0}"
    [[ "$actual" -eq "$expected" ]] || {
        printf 'Expected %s occurrences of %s in %s; found %s\n' \
            "$expected" "$needle" "$workflow" "$actual" >&2
        exit 1
    }
}

require_count 'brew install cmake qtbase qtsvg opencv@4 boost libusb protobuf abseil gcc' 3
require_count 'OPENCV_PREFIX="$(brew --prefix opencv@4)"' 3
require_count '$(brew --prefix opencv@4);$(brew --prefix)' 1
