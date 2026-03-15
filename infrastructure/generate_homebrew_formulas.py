#!/usr/bin/env python3
"""
Generate Homebrew formulas using Bazel-generated config.

Usage:
    bazel run //infrastructure:generate-homebrew-formulas -- /path/to/homebrew-mull
"""

import json
import os
import re
import sys
from pathlib import Path

CHANNELS = ["stable", "nightly", "testing"]


def read_existing_values(formula_path):
    """Read version, url, and sha256 from existing formula if not placeholders."""
    if not formula_path.exists():
        return None, None, None

    content = formula_path.read_text()

    version_match = re.search(r'version "([^"]+)"', content)
    url_match = re.search(r'url "([^"]+)"', content)
    sha256_match = re.search(r'sha256 "([^"]+)"', content)

    version = version_match.group(1) if version_match else None
    url = url_match.group(1) if url_match else None
    sha256 = sha256_match.group(1) if sha256_match else None

    # Return None for placeholders
    if url and "PLACEHOLDER" in url:
        url = None
    if sha256 and "PLACEHOLDER" in sha256:
        sha256 = None

    return version, url, sha256

FORMULA_TEMPLATE = '''\
class {class_name} < Formula
  desc "Practical mutation testing for C and C++{channel_desc} (LLVM {llvm_version})"
  homepage "https://github.com/mull-project/mull"
  version "{mull_version}"
  license "Apache-2.0"
{livecheck}
  depends_on "{llvm_dep}"
  depends_on :macos

  on_macos do
    on_arm do
      url "{url}"
      sha256 "{sha256}"
    end
  end

{conflicts}
  def install
    bin.install "bin/mull-runner-{llvm_version}"
    bin.install "bin/mull-reporter-{llvm_version}"
    lib.install "lib/mull-ir-frontend-{llvm_version}"
    man1.install "share/man/man1/mull-runner-{llvm_version}.1"
    man1.install "share/man/man1/mull-reporter-{llvm_version}.1"
    bash_completion.install "share/bash-completion/completions/mull-runner-{llvm_version}"
    bash_completion.install "share/bash-completion/completions/mull-reporter-{llvm_version}"
    zsh_completion.install "share/zsh/site-functions/_mull-runner-{llvm_version}"
    zsh_completion.install "share/zsh/site-functions/_mull-reporter-{llvm_version}"
    fish_completion.install "share/fish/vendor_completions.d/mull-runner-{llvm_version}.fish"
    fish_completion.install "share/fish/vendor_completions.d/mull-reporter-{llvm_version}.fish"
  end

  def caveats
    <<~EOS
      Mull{channel_caveat} has been installed with LLVM {llvm_version} support.
{unstable_warning}
      Binaries installed:
        mull-runner-{llvm_version}
        mull-reporter-{llvm_version}

      The Clang plugin is installed at:
        #{{lib}}/mull-ir-frontend-{llvm_version}

      Usage with clang:
        clang -fpass-plugin=#{{lib}}/mull-ir-frontend-{llvm_version} ...
    EOS
  end

  test do
    assert_match "mull-runner", shell_output("#{{bin}}/mull-runner-{llvm_version} --version")
  end
end
'''


def load_config():
    """Load config from Bazel-generated JSON."""
    # bazel run sets RUNFILES_DIR
    runfiles = os.environ.get("RUNFILES_DIR", "")
    config_path = Path(runfiles) / "mull" / "infrastructure" / "homebrew_config.json"

    if not config_path.exists():
        # Fallback: check bazel-bin
        workspace = os.environ.get("BUILD_WORKSPACE_DIRECTORY", "")
        config_path = Path(workspace) / "bazel-bin" / "infrastructure" / "homebrew_config.json"

    with open(config_path) as f:
        return json.load(f)


def formula_prefix(channel):
    return "mull" if channel == "stable" else f"mull-{channel}"


def class_name_for(channel, llvm_version):
    if channel == "stable":
        return f"MullAT{llvm_version}"
    return f"Mull{channel.title()}AT{llvm_version}"


def generate_conflicts(current_version, current_channel):
    lines = []
    for channel in CHANNELS:
        if channel == current_channel:
            continue
        prefix = formula_prefix(channel)
        lines.append(f'  conflicts_with "{prefix}@{current_version}", because: "both install mull-runner-{current_version}"')
    return "\n".join(lines)


def generate_formula(llvm_version, mull_version, channel, existing_version=None, existing_url=None, existing_sha256=None):
    if channel == "stable":
        channel_desc, channel_caveat, unstable_warning, livecheck = "", "", "", ""
        version_str = mull_version
    elif channel == "nightly":
        channel_desc = " - Nightly"
        channel_caveat = " NIGHTLY"
        unstable_warning = "\n      This is a development build and may be unstable.\n"
        livecheck = '\n  livecheck do\n    skip "Nightly builds are updated automatically"\n  end\n'
        version_str = f"{mull_version}.trunk1"
    else:
        channel_desc = " - Testing"
        channel_caveat = " TESTING"
        unstable_warning = "\n      This is a PR/testing build and may be unstable.\n"
        livecheck = '\n  livecheck do\n    skip "Testing builds are updated automatically"\n  end\n'
        version_str = f"{mull_version}.pr1"

    llvm_dep = f"llvm@{llvm_version}"

    # Use existing values if available, otherwise use placeholders
    final_version = existing_version or version_str
    final_url = existing_url or f"https://dl.cloudsmith.io/public/mull-project/mull-{channel}/raw/names/mull-{llvm_version}/versions/{version_str}/PACKAGE_FILENAME_PLACEHOLDER"
    final_sha256 = existing_sha256 or "PLACEHOLDER_SHA256"

    return FORMULA_TEMPLATE.format(
        class_name=class_name_for(channel, llvm_version),
        llvm_version=llvm_version,
        mull_version=final_version,
        llvm_dep=llvm_dep,
        url=final_url,
        sha256=final_sha256,
        conflicts=generate_conflicts(llvm_version, channel),
        channel_desc=channel_desc,
        channel_caveat=channel_caveat,
        unstable_warning=unstable_warning,
        livecheck=livecheck,
    )


def main():
    if len(sys.argv) < 2:
        print("Usage: bazel run //infrastructure:generate-homebrew-formulas -- /path/to/homebrew-mull")
        sys.exit(1)

    output_dir = Path(sys.argv[1]).resolve()
    formula_dir = output_dir / "Formula"

    config = load_config()
    mull_version = config["mull_version"]
    llvm_versions = [int(v) for v in config["llvm_versions"]]

    print(f"Mull version: {mull_version}")
    print(f"LLVM versions: {llvm_versions}")

    formula_dir.mkdir(exist_ok=True)

    for llvm_version in llvm_versions:
        for channel in CHANNELS:
            prefix = formula_prefix(channel)
            filename = f"{prefix}@{llvm_version}.rb"
            formula_path = formula_dir / filename

            # Read existing values to preserve them
            existing_version, existing_url, existing_sha256 = read_existing_values(formula_path)

            formula_path.write_text(
                generate_formula(llvm_version, mull_version, channel,
                                existing_version, existing_url, existing_sha256)
            )
            preserved = " (preserved)" if existing_url else " (new)"
            print(f"  {filename}{preserved}")


if __name__ == "__main__":
    main()
