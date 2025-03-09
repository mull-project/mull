#!/usr/bin/env python3

from lit.main import main

import os

if __name__ == "__main__":
    # `lit` doesn't find the config since by default Bazel sets cwd to be runfiles directory
    # Changing cwd so that lit test works
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    print(f"Running lit test from {script_dir}")

    main()
