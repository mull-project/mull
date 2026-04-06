#!/bin/bash
# Runs mull-runner while another process holds an exclusive SQLite lock,
# reproducing the "database is locked" race condition.
# Usage: run.sh <python3> <hold_lock.py> <mull_runner> <db_path> <lock_duration> <test_exe> <busy_timeout_ms>
PYTHON3=$1
HOLD_LOCK=$2
MULL_RUNNER=$3
DB=$4
LOCK_DURATION=$5
TEST_EXE=$6
BUSY_TIMEOUT_MS=${7:-5000}

READY_FILE="$(dirname "$DB")/lock.ready"
rm -f "$READY_FILE"

"$PYTHON3" "$HOLD_LOCK" "$DB" "$READY_FILE" "$LOCK_DURATION" &
LOCK_PID=$!

# Wait until the lock is actually held before starting mull-runner
until [ -f "$READY_FILE" ]; do sleep 0.05; done

"$MULL_RUNNER" --allow-surviving --reporters=SQLite \
    --report-dir="$(dirname "$DB")" \
    --report-name="$(basename "$DB" .sqlite)" \
    --sqlite-busy-timeout="$BUSY_TIMEOUT_MS" \
    "$TEST_EXE"
MULL_EXIT=$?

wait $LOCK_PID
exit $MULL_EXIT
