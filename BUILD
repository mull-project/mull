load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "libmull_18",
    srcs = glob(["lib/**/*.cpp"]),
    hdrs = glob(["include/**/*.h"]),
    includes = ["include"],
    deps = [
        "@irm",
        "@json11",
        "@llvm_18//:libclang",
        "@llvm_18//:libllvm",
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
        "@llvm_18//:libllvm",
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

cc_library(
    name = "libmull_reporter_18",
    srcs = glob(["tools/mull-reporter/*.cpp"]),
    hdrs = glob(["tools/mull-reporter/*.h"]),
    deps = [
        "libmull_18",
        ":libmull_cli_options_18",
    ],
)

cc_binary(
    name = "mull-cxx-ir-frontend-18",
    srcs = glob(["tools/mull-ir-frontend/*.cpp"]),
    linkshared = True,
    deps = [
        ":libmull_18",
        "@llvm_18//:libclang",
    ],
)

genrule(
    name = "mull-ir-frontend-18-gen",
    srcs = [":mull-cxx-ir-frontend-18"],
    outs = ["mull-ir-frontend-18"],
    cmd = "cp $(SRCS) $(OUTS)",
)

cc_binary(
    name = "mull-cxx-ast-frontend-18",
    srcs = glob([
        "tools/mull-cxx-frontend/src/*.cpp",
        "tools/mull-cxx-frontend/src/*.h",
    ]),
    linkshared = True,
    deps = [
        ":libmull_18",
        "@llvm_18//:libclang",
    ],
)

genrule(
    name = "mull-ast-frontend-18-gen",
    srcs = [":mull-cxx-ast-frontend-18"],
    outs = ["mull-ast-frontend-18"],
    cmd = "cp $(SRCS) $(OUTS)",
)

cc_binary(
    name = "mull_runner_18",
    deps = [
        ":libmull_runner_18",
    ],
)

cc_binary(
    name = "mull_reporter_18",
    deps = [
        ":libmull_reporter_18",
    ],
)

cc_test(
    name = "mull_unit_tests",
    size = "small",
    srcs = [
        "tests/unit/MutatorsFactoryTests.cpp",
        "tests/unit/TaskExecutorTests.cpp",
    ],
    deps = [
        ":libmull_18",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
