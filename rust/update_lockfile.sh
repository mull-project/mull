#!/usr/bin/env bash
set -euo pipefail

CARGO="$1"
"$CARGO" update --manifest-path "$BUILD_WORKSPACE_DIRECTORY/rust/Cargo.toml" "${@:2}"
