"""Applies each patch to a clean copy of the sources and compiles the result"""

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile

MUTATOR = re.compile(r"-((?:cxx|negate)_[a-z_]+)-L\d+-C\d+\.patch$")

parser = argparse.ArgumentParser()
parser.add_argument("--patches", required=True)
parser.add_argument("--sources", required=True, help="directory the patch paths are relative to")
parser.add_argument("--compiler", required=True, help="compiler command, may include flags")
args = parser.parse_args()

failures = 0
mutators = set()
for name in sorted(os.listdir(args.patches)):
    patch = os.path.join(args.patches, name)
    match = MUTATOR.search(name)
    if not match:
        print(f"FAIL {name}: cannot find the mutator in the name")
        failures += 1
        continue
    with tempfile.TemporaryDirectory() as work:
        tree = os.path.join(work, "tree")
        shutil.copytree(args.sources, tree)
        # Keep git from treating an enclosing repository as the patch root
        env = dict(os.environ, GIT_CEILING_DIRECTORIES=work)
        applied = subprocess.run(
            ["git", "apply", "--unidiff-zero", "--verbose", patch],
            cwd=tree, env=env, capture_output=True, text=True,
        )
        if applied.returncode != 0:
            print(f"FAIL {name}: does not apply\n{applied.stderr}")
            failures += 1
            continue
        changed = subprocess.run(
            ["git", "diff", "--no-index", "--quiet", args.sources, tree], env=env
        )
        if changed.returncode == 0:
            print(f"FAIL {name}: the patch changes nothing")
            failures += 1
            continue
        for root, _, files in os.walk(tree):
            for source in files:
                if not source.endswith((".c", ".cpp")):
                    continue
                compiled = subprocess.run(
                    shlex.split(args.compiler) + ["-fsyntax-only", os.path.join(root, source)],
                    capture_output=True, text=True,
                )
                if compiled.returncode != 0:
                    print(f"FAIL {name}: the mutated source does not compile\n{compiled.stderr}")
                    failures += 1
    mutators.add(match.group(1))

for mutator in sorted(mutators):
    print(f"MUTATOR: {mutator}")
print(f"FAILURES: {failures}")
sys.exit(1 if failures else 0)
