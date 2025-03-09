load("@bazel_skylib//rules:native_binary.bzl", "native_binary")
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

native_binary(
    name = "python3",
    src = "@python_3_13//:python3",
    out = "python3",
    visibility = ["//visibility:public"],
)

[mull_py_test(src = f) for f in glob([
    "**/*.cpp",
    "**/*.c",
    "**/*.itest",
])]
