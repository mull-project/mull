load("@rules_cc//cc:defs.bzl", "cc_library")

[cc_library(
    name = "irm%s" % v,
    srcs = glob([
        "lib/**/*.cpp",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@llvm_%s//:libllvm" % v,
    ],
) for v in [
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
]]
