#!/usr/bin/env bash
set -euo pipefail

CARGO="$1"
CARGO_TOML="$2"
CARGO_LOCK="$3"

if ! "$CARGO" metadata --manifest-path "$CARGO_TOML" --locked --format-version 1 > /dev/null 2>&1; then
    echo "Cargo.lock is out of sync with Cargo.toml. Run the following to update it:"
    echo "  bazel run //rust:update_cargo_lockfile"
    exit 1
fi
