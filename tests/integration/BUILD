load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_binary")
load("//tests/integration:mull_integration_test.bzl", "mull_py_test")

py_binary(
    name = "filecheck_runner",
    srcs = ["filecheck_runner.py"],
    deps = [
        requirement("filecheck"),
    ],
)

[mull_py_test(src = f) for f in glob(["**/*.cpp"])]

# mull_py_test(
#     src = "ast-search/arithmetic/01_add_to_sub/sample.cpp",
# )
