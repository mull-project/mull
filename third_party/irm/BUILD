load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "irm",
    srcs = glob([
        "lib/**/*.cpp",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@llvm_18",
    ],
)
