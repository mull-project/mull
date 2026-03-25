#!/usr/bin/env bash
set -euo pipefail

MODULE_BAZEL="$1"
CARGO_TOML="$2"

CARGO_EDITION=$(grep '^edition' "$CARGO_TOML" | sed 's/edition = "\(.*\)"/\1/')
CARGO_RUST_VERSION=$(grep '^rust-version' "$CARGO_TOML" | sed 's/rust-version = "\(.*\)"/\1/')

BAZEL_EDITION=$(grep 'edition = ' "$MODULE_BAZEL" | sed 's/.*edition = "\(.*\)".*/\1/')
BAZEL_RUST_VERSION=$(grep 'versions = \[' "$MODULE_BAZEL" | sed 's/.*"\(.*\)".*/\1/')

FAILED=0

if [ "$CARGO_EDITION" != "$BAZEL_EDITION" ]; then
    echo "Edition mismatch: rust/Cargo.toml='$CARGO_EDITION', MODULE.bazel='$BAZEL_EDITION'"
    FAILED=1
fi

if [ "$CARGO_RUST_VERSION" != "$BAZEL_RUST_VERSION" ]; then
    echo "Rust version mismatch: rust/Cargo.toml='$CARGO_RUST_VERSION', MODULE.bazel='$BAZEL_RUST_VERSION'"
    FAILED=1
fi

if [ "$FAILED" -eq 1 ]; then
    echo "Keep rust/Cargo.toml and MODULE.bazel in sync."
    exit 1
fi
