#!/usr/bin/env python3

from lit.main import main

import os
import sys

if __name__ == "__main__":
    # `lit` doesn't find the config since by default Bazel sets cwd to be runfiles directory
    # Changing cwd so that lit test works
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    print(f"Running lit test from {script_dir}")

    # Time out before Bazel does, so that lit reports the output of a hanging test
    bazel_timeout = int(os.environ.get("TEST_TIMEOUT", "60"))
    sys.argv.append(f"--timeout={max(bazel_timeout - 10, 1)}")

    main()
