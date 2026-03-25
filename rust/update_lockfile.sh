#!/usr/bin/env bash
set -euo pipefail

CARGO="$1"
"$CARGO" update --manifest-path "$BUILD_WORKSPACE_DIRECTORY/rust/Cargo.toml" "${@:2}"

echo "Cargo.lock updated. Run the following to repin the Bazel crate universe:"
echo "  CARGO_BAZEL_REPIN=1 bazel build //..."
