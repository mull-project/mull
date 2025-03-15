load("@pypi//:requirements.bzl", "requirement")

# load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:defs.bzl", "py_binary", "py_test")

py_binary(
    name = "filecheck_runner",
    srcs = ["filecheck_runner.py"],
    deps = [
        requirement("filecheck"),
    ],
)

py_test(
    name = "a_test",
    srcs = ["lit_runner.py"],
    args = [
        "-v",
        "tutorials/hello-world/step-2-1-no-debug-no-config/sample.cpp",
    ],
    data = [
        "tutorials/hello-world/step-2-1-no-debug-no-config/sample.cpp",
        ":lit.cfg.py",
        requirement("lit"),
        ":filecheck_runner",
        "//:mull-ast-frontend-18-gen",
        "//:mull-ir-frontend-18-gen",
        "@llvm_18//:clang",
        "@llvm_18//:clangxx",
        "@llvm_18//:llvm-profdata",
    ],
    env = {
        "LLVM_VERSION_MAJOR": "18",
    },
    main = "lit_runner.py",
    deps = [
        requirement("lit"),
        ":filecheck_runner",
        # "//:mull-cxx-ir-frontend-18",
        "//:mull_runner_18",
        "@rules_python//python/runfiles",
    ],
)
