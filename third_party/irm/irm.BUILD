load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "irm17",
    srcs = glob([
        "lib/**/*.cpp",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@llvm_17//:libllvm",
    ],
)

cc_library(
    name = "irm18",
    srcs = glob([
        "lib/**/*.cpp",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@llvm_18//:libllvm",
    ],
)
