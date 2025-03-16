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

    # test_support_files = () for f in test_support_files
    py_test(
        name = "%s_test" % src,
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
            "@llvm_18//:clang",
            "@llvm_18//:clangxx",
            "@llvm_18//:llvm-profdata",
            "//:mull-ast-frontend-18-gen",
            "//:mull-ir-frontend-18-gen",
            "//:mull_runner_18",
            "//:mull_reporter_18",
        ] + native.glob(test_support_files, allow_empty = True),
        env = {
            "LLVM_VERSION_MAJOR": "18",
        },
        main = "lit_runner.py",
        deps = [
            requirement("lit"),
            "@rules_python//python/runfiles",
        ],
    )
