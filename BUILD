load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "libmull_18",
    srcs = glob(["lib/**/*.cpp"]),
    hdrs = glob(["include/**/*.h"]),
    includes = ["include"],
    deps = [
        "@irm",
        "@json11",
        "@llvm_18",
        "@llvm_18//:clang_18",
        "@reproc//:reproc++",
        "@spdlog",
        "@sqlite3",
    ],
)

cc_library(
    name = "libmull_cli_options_18",
    srcs = ["tools/CLIOptions/CLIOptions.cpp"],
    hdrs = ["tools/CLIOptions/CLIOptions.h"],
    deps = [
        ":libmull_18",
        "@llvm_18",
    ],
)

cc_library(
    name = "libmull_runner_18",
    srcs = glob(["tools/mull-runner/*.cpp"]),
    hdrs = glob(["tools/mull-runner/*.h"]),
    deps = [
        "libmull_18",
        ":libmull_cli_options_18",
    ],
)

cc_binary(
    name = "mull_runner_18",
    deps = [
        ":libmull_runner_18",
    ],
)
