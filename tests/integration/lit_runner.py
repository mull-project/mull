#!/usr/bin/env python3

from lit.main import main

import os
import psutil
import subprocess
import sys
import threading


def dump_hanging_processes():
    # Runs well before lit's timeout, so anything still alive here is likely stuck
    children = psutil.Process().children(recursive=True)
    if not children:
        return
    pids = ",".join(str(p.pid) for p in children)
    print("=== Hanging test processes ===", file=sys.stderr)
    ps = subprocess.run(
        ["ps", "-o", "pid,ppid,stat,wchan,etime,command", "-p", pids],
        capture_output=True,
        text=True,
    )
    print(ps.stdout, file=sys.stderr)
    if sys.platform != "darwin":
        return
    for child in children:
        try:
            if "python" in child.name().lower():
                continue
            sample = subprocess.run(
                ["sample", str(child.pid), "1"],
                capture_output=True,
                text=True,
                timeout=10,
            )
            print(f"=== sample {child.pid} ===\n{sample.stdout}", file=sys.stderr)
        except subprocess.TimeoutExpired:
            print(f"=== sample {child.pid} timed out ===", file=sys.stderr)
        except psutil.NoSuchProcess:
            pass
    sys.stderr.flush()

if __name__ == "__main__":
    # `lit` doesn't find the config since by default Bazel sets cwd to be runfiles directory
    # Changing cwd so that lit test works
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    print(f"Running lit test from {script_dir}")

    # Time out before Bazel does, so that lit reports the output of a hanging test
    bazel_timeout = int(os.environ.get("TEST_TIMEOUT", "60"))
    sys.argv.append(f"--timeout={max(bazel_timeout - 10, 1)}")

    watchdog = threading.Timer(max(bazel_timeout - 25, 1), dump_hanging_processes)
    watchdog.daemon = True
    watchdog.start()

    main()
