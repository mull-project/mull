#!/bin/bash

# Workspace status script for Bazel stamping
# Outputs key-value pairs for embedding in binaries

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(dirname "$SCRIPT_DIR")"

# Base version from MODULE.bazel
BASE_VERSION=$(grep -E '^\s+version\s*=' "$WORKSPACE_ROOT/MODULE.bazel" | head -1 | sed 's/.*"\([^"]*\)".*/\1/')

# Version suffix from CI (if exists)
SUFFIX_FILE="$WORKSPACE_ROOT/.mull-ci-version-suffix"
if [[ -f "$SUFFIX_FILE" ]]; then
    SUFFIX=$(cat "$SUFFIX_FILE")
else
    SUFFIX=""
fi

# Full version
FULL_VERSION="${BASE_VERSION}${SUFFIX}"

# STABLE_ prefix means cache invalidation when value changes
echo "STABLE_MULL_VERSION ${FULL_VERSION}"
echo "STABLE_MULL_BASE_VERSION ${BASE_VERSION}"
