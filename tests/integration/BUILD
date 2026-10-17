load("@bazel_skylib//rules:native_binary.bzl", "native_binary")
load("//tests/integration:mull_integration_test.bzl", "mull_py_test")

[config_setting(
    name = name,
    constraint_values = constraints,
) for name, constraints in {
    "darwin_aarch64": [
        "@platforms//os:macos",
        "@platforms//cpu:aarch64",
    ],
    "linux_aarch64": [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
    "linux_x86_64": [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
}.items()]

native_binary(
    name = "filecheck",
    src = select({
        ":darwin_aarch64": "@filecheck_darwin_aarch64//:bin/FileCheck",
        ":linux_aarch64": "@filecheck_linux_aarch64//:bin/FileCheck",
        ":linux_x86_64": "@filecheck_linux_x86_64//:bin/FileCheck",
    }),
    out = "FileCheck",
    visibility = ["//visibility:public"],
)

native_binary(
    name = "python3",
    src = "@python_3_14//:python3",
    out = "python3",
    visibility = ["//visibility:public"],
)

native_binary(
    name = "jq",
    src = "@jq",
    out = "jq",
    visibility = ["//visibility:public"],
)

native_binary(
    name = "sqlite3",
    src = "@sqlite3//:shell",
    out = "sqlite3",
    visibility = ["//visibility:public"],
)

[mull_py_test(src = f) for f in glob([
    "**/*.cpp",
    "**/*.c",
    "**/*.itest",
])]
