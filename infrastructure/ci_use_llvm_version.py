#!/usr/bin/env python3
"""Configures MODULE.bazel to build only one LLVM version on one OS, e.g. an unreleased one.

Usage: ci_use_llvm_version.py <MODULE.bazel> <os key, e.g. ubuntu:24.04> <LLVM major version>
"""

import re
import sys


def main():
    path, os_key, version = sys.argv[1:]
    text = open(path).read()

    supported = re.search(r"SUPPORTED_LLVM_VERSIONS = \[(.*?)\]", text, re.S)
    if not supported:
        sys.exit("SUPPORTED_LLVM_VERSIONS not found in " + path)
    versions = re.findall(r'"(\d+)"', supported.group(1))
    if version not in versions:
        versions.append(version)
    listed = "".join('    "%s",\n' % v for v in versions)
    text = text[: supported.start()] + "SUPPORTED_LLVM_VERSIONS = [\n" + listed + "]" + text[supported.end() :]

    mapping = re.search(r'"%s": \[.*?\]' % re.escape(os_key), text, re.S)
    if not mapping:
        sys.exit("%s not found in the os_version_mapping of %s" % (os_key, path))
    text = text[: mapping.start()] + '"%s": ["%s"]' % (os_key, version) + text[mapping.end() :]

    open(path, "w").write(text)


if __name__ == "__main__":
    main()
