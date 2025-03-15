load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_test")

def _dirname(filepath):
    parts = filepath.rsplit("/", 1)  # Split at the last '/'
    return parts[0] if len(parts) > 1 else "."

def mull_py_test(src):
    """
    A wrapper around py_test to enforce conventions and add defaults.
    """
    py_test(
        name = "%s_test" % src,
        srcs = ["lit_runner.py"],
        args = [
            "-v",
            src,
        ],
        data = [
                   src,
                   requirement("lit"),
                   ":lit.cfg.py",
                   ":filecheck_runner",
                   "@llvm_18//:clang",
                   "@llvm_18//:clangxx",
                   "@llvm_18//:llvm-profdata",
                   "//:mull-ast-frontend-18-gen",
                   "//:mull-ir-frontend-18-gen",
                   "//:mull_runner_18",
                   "//:mull_reporter_18",
               ] + native.glob(["%s/*.yml" % _dirname(src)], allow_empty = True) +
               native.glob(["%s/*.json.template" % _dirname(src)], allow_empty = True) +
               native.glob(["%s/*.modified" % _dirname(src)], allow_empty = True) +
               native.glob(["%s/*.original" % _dirname(src)], allow_empty = True) +
               native.glob(["%s/*.h" % _dirname(src)], allow_empty = True),
        env = {
            "LLVM_VERSION_MAJOR": "18",
        },
        main = "lit_runner.py",
        deps = [
            requirement("lit"),
            "@rules_python//python/runfiles",
        ],
    )
