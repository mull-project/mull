load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_test")

def _dirname(filepath):
    parts = filepath.rsplit("/", 1)  # Split at the last '/'
    return parts[0] if len(parts) > 1 else "."

def mull_py_test(src):
    """
    A wrapper around py_test to enforce conventions and add defaults.
    """
    test_dir = _dirname(src)
    test_support_files = [
        "%s/*.yml" % test_dir,
        "%s/*.json.template" % test_dir,
        "%s/*.modified" % test_dir,
        "%s/*.original" % test_dir,
        "%s/*.py" % test_dir,
        "%s/*.h" % test_dir,
        "%s/*.hh" % test_dir,
        "%s/*.cpp" % test_dir,
        "%s/*.c" % test_dir,
        "%s/*.notest" % test_dir,
        "%s/*.itest" % test_dir,
    ]
    for version in AVAILABLE_LLVM_VERSIONS:
        py_test(
            name = "%s_%s_test" % (src, version),
            srcs = ["lit_runner.py"],
            args = [
                "-v",
                src,
            ],
            data = [
                requirement("lit"),
                ":lit.cfg.py",
                ":filecheck_runner",
                ":python3",
                "@llvm_%s//:clang" % version,
                "@llvm_%s//:clangxx" % version,
                "@llvm_%s//:llvm-profdata" % version,
                "//:mull-ast-frontend-%s-gen" % version,
                "//:mull-ir-frontend-%s-gen" % version,
                "//:mull_runner_%s" % version,
                "//:mull_reporter_%s" % version,
            ] + native.glob(test_support_files, allow_empty = True),
            env = {
                "LLVM_VERSION_MAJOR": version,
            },
            main = "lit_runner.py",
            deps = [
                requirement("lit"),
                "@rules_python//python/runfiles",
            ],
        )
