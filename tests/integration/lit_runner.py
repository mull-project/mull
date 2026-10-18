#!/usr/bin/env python3

from lit.main import main

import os
import subprocess
import sys
import time


def watchdog(target_pid, delay):
    # Separate process: dumps the lit process tree even when lit itself is frozen
    import psutil

    deadline = time.monotonic() + delay
    while time.monotonic() < deadline:
        if not psutil.pid_exists(target_pid):
            return
        time.sleep(1)
    try:
        target = psutil.Process(target_pid)
        processes = [target] + target.children(recursive=True)
    except psutil.NoSuchProcess:
        return
    processes = [p for p in processes if p.pid != os.getpid()]
    pids = ",".join(str(p.pid) for p in processes)
    ps = subprocess.run(
        ["ps", "-o", "pid,ppid,stat,wchan,etime,command", "-p", pids],
        capture_output=True,
        text=True,
    )
    print(f"=== Hanging test processes ===\n{ps.stdout}", file=sys.stderr)
    samples = [
        (p.pid, subprocess.Popen(["sample", str(p.pid), "1"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True))
        for p in processes
    ]
    for pid, sample in samples:
        try:
            output, _ = sample.communicate(timeout=10)
            print(f"=== sample {pid} ===\n{output}", file=sys.stderr)
        except subprocess.TimeoutExpired:
            sample.kill()
            print(f"=== sample {pid} timed out ===", file=sys.stderr)
    sys.stderr.flush()


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "--watchdog":
        watchdog(int(sys.argv[2]), float(sys.argv[3]))
        sys.exit(0)

    # `lit` doesn't find the config since by default Bazel sets cwd to be runfiles directory
    # Changing cwd so that lit test works
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    print(f"Running lit test from {script_dir}")

    # Time out before Bazel does, so that lit reports the output of a hanging test
    bazel_timeout = int(os.environ.get("TEST_TIMEOUT", "60"))
    sys.argv.append(f"--timeout={max(bazel_timeout - 10, 1)}")

    # Tests only hang on macOS
    watchdog_process = None
    if sys.platform == "darwin":
        watchdog_process = subprocess.Popen(
            [sys.executable, os.path.abspath(__file__), "--watchdog", str(os.getpid()), str(max(bazel_timeout - 25, 1))]
        )

    try:
        main()
    finally:
        if watchdog_process:
            watchdog_process.kill()
