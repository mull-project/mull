#!/usr/bin/env python3
"""
Generate Homebrew formulas from Bazel config.

Reads version and LLVM versions from MODULE.bazel - the single source of truth.

Usage:
    bazel run //tools:generate-homebrew-formulas -- /path/to/homebrew-mull
    # or directly:
    ./tools/generate_homebrew_formulas.py /path/to/homebrew-mull
"""

import os
import re
import sys
from pathlib import Path

# Channels: stable, nightly, testing
CHANNELS = ["stable", "nightly", "testing"]

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
    bin.install "usr/local/bin/mull-runner-{llvm_version}"
    bin.install "usr/local/bin/mull-reporter-{llvm_version}"
    lib.install "usr/local/lib/mull-ir-frontend-{llvm_version}"
    man1.install "usr/local/share/man/man1/mull-runner-{llvm_version}.1"
    man1.install "usr/local/share/man/man1/mull-reporter-{llvm_version}.1"
    bash_completion.install "usr/local/share/bash-completion/completions/mull-runner-{llvm_version}"
    bash_completion.install "usr/local/share/bash-completion/completions/mull-reporter-{llvm_version}"
    zsh_completion.install "usr/local/share/zsh/site-functions/_mull-runner-{llvm_version}"
    zsh_completion.install "usr/local/share/zsh/site-functions/_mull-reporter-{llvm_version}"
    fish_completion.install "usr/local/share/fish/vendor_completions.d/mull-runner-{llvm_version}.fish"
    fish_completion.install "usr/local/share/fish/vendor_completions.d/mull-reporter-{llvm_version}.fish"
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


def find_repo_root():
    """Find the mull repo root by looking for MODULE.bazel."""
    # When run via bazel, we're in a runfiles directory
    # Try BUILD_WORKSPACE_DIRECTORY first (set by `bazel run`)
    workspace = os.environ.get("BUILD_WORKSPACE_DIRECTORY")
    if workspace:
        return Path(workspace)

    # Otherwise, walk up from script location
    path = Path(__file__).resolve()
    while path.parent != path:
        if (path / "MODULE.bazel").exists():
            return path
        path = path.parent
    raise RuntimeError("Could not find repo root (MODULE.bazel)")


def parse_module_bazel(content):
    """Extract version and macOS LLVM versions from MODULE.bazel."""
    # Extract version
    version_match = re.search(r'module\([^)]*version\s*=\s*"([^"]+)"', content, re.DOTALL)
    if not version_match:
        raise ValueError("Could not find module version in MODULE.bazel")
    mull_version = version_match.group(1)

    # Extract macOS LLVM versions from os_version_mapping
    # Look for: "macos": ["16", "17", ...]
    macos_match = re.search(r'"macos"\s*:\s*\[([\s\S]*?)\]', content)
    if not macos_match:
        raise ValueError("Could not find macos LLVM versions in MODULE.bazel")

    versions_str = macos_match.group(1)
    llvm_versions = [int(v.strip().strip('"')) for v in versions_str.split(",") if v.strip()]

    return mull_version, llvm_versions


def formula_prefix(channel):
    """Get formula name prefix for a channel."""
    if channel == "stable":
        return "mull"
    return f"mull-{channel}"


def class_name_for(channel, llvm_version):
    """Get Ruby class name for a formula."""
    if channel == "stable":
        return f"MullAT{llvm_version}"
    return f"Mull{channel.title()}AT{llvm_version}"


def generate_conflicts(llvm_versions, current_version, current_channel):
    """Generate conflicts_with statements.

    Only same LLVM version across different channels conflicts,
    because they install the same versioned binaries (e.g., mull-runner-19).
    Different LLVM versions can coexist (mull-runner-16 vs mull-runner-19).
    """
    lines = []
    for channel in CHANNELS:
        if channel == current_channel:
            continue
        prefix = formula_prefix(channel)
        lines.append(f'  conflicts_with "{prefix}@{current_version}", because: "both install mull-runner-{current_version}"')
    return "\n".join(lines)


def generate_formula(llvm_version, llvm_versions, mull_version, channel):
    """Generate a single formula."""
    llvm_latest = max(llvm_versions)

    class_name = class_name_for(channel, llvm_version)
    prefix = formula_prefix(channel)

    if channel == "stable":
        channel_desc = ""
        channel_caveat = ""
        unstable_warning = ""
        livecheck = ""
        version_str = mull_version
    elif channel == "nightly":
        channel_desc = " - Nightly"
        channel_caveat = " NIGHTLY"
        unstable_warning = "\n      This is a development build and may be unstable.\n"
        livecheck = '''
  livecheck do
    skip "Nightly builds are updated automatically"
  end
'''
        version_str = f"{mull_version}.trunk1"
    else:  # testing
        channel_desc = " - Testing"
        channel_caveat = " TESTING"
        unstable_warning = "\n      This is a PR/testing build and may be unstable.\n"
        livecheck = '''
  livecheck do
    skip "Testing builds are updated automatically"
  end
'''
        version_str = f"{mull_version}.pr1"

    llvm_dep = "llvm" if llvm_version == llvm_latest else f"llvm@{llvm_version}"
    url = f"https://dl.cloudsmith.io/public/mull-project/mull-{channel}/raw/names/mull-{llvm_version}/versions/{version_str}/PACKAGE_FILENAME_PLACEHOLDER"
    conflicts = generate_conflicts(llvm_versions, llvm_version, channel)

    return FORMULA_TEMPLATE.format(
        class_name=class_name,
        llvm_version=llvm_version,
        mull_version=version_str,
        llvm_dep=llvm_dep,
        url=url,
        sha256="PLACEHOLDER_SHA256",
        conflicts=conflicts,
        channel_desc=channel_desc,
        channel_caveat=channel_caveat,
        unstable_warning=unstable_warning,
        livecheck=livecheck,
    )


def main():
    if len(sys.argv) < 2:
        print("Usage: generate_homebrew_formulas.py <homebrew-mull-repo-path>")
        print()
        print("Example:")
        print("  bazel run //tools:generate-homebrew-formulas -- ../homebrew-mull")
        sys.exit(1)

    output_dir = Path(sys.argv[1]).resolve()
    formula_dir = output_dir / "Formula"

    if not output_dir.exists():
        print(f"Error: {output_dir} does not exist")
        sys.exit(1)

    # Find and parse MODULE.bazel
    repo_root = find_repo_root()
    module_bazel = repo_root / "MODULE.bazel"
    content = module_bazel.read_text()
    mull_version, llvm_versions = parse_module_bazel(content)

    print(f"Mull version: {mull_version}")
    print(f"LLVM versions (macOS): {llvm_versions}")
    print()

    # Generate formulas
    formula_dir.mkdir(exist_ok=True)
    generated = []

    for llvm_version in llvm_versions:
        for channel in CHANNELS:
            prefix = formula_prefix(channel)
            filename = f"{prefix}@{llvm_version}.rb"
            filepath = formula_dir / filename
            content = generate_formula(llvm_version, llvm_versions, mull_version, channel)
            filepath.write_text(content)
            generated.append(filename)

    print(f"Generated {len(generated)} formulas in {formula_dir}:")
    for name in sorted(generated):
        print(f"  - {name}")


if __name__ == "__main__":
    main()
