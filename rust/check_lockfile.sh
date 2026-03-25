#!/usr/bin/env bash
set -euo pipefail

CARGO="$1"
RUSTC="$2"
CARGO_TOML="$3"
CARGO_LOCK="$4"

export RUSTC

echo "cargo: $("$CARGO" --version)"
echo "rustc: $("$RUSTC" --version)"
echo "Cargo.toml: $CARGO_TOML"
echo "Cargo.lock: $CARGO_LOCK"

if ! "$CARGO" metadata --manifest-path "$CARGO_TOML" --locked --format-version 1 > /dev/null; then
    echo "Cargo.lock is out of sync with Cargo.toml. Run the following to update it:"
    echo "  bazel run //rust:update_cargo_lockfile"
    exit 1
fi
