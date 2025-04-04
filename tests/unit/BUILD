load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_cc//cc:defs.bzl", "cc_test")

[
    cc_test(
        name = "mull_unit_tests_%s" % llvm_version,
        size = "small",
        srcs = [
            "MutationFilters/GitDiffReaderTests.cpp",
            "MutatorsFactoryTests.cpp",
            "TaskExecutorTests.cpp",
        ],
        deps = [
            "//:libmull_%s" % llvm_version,
            "@googletest//:gtest",
            "@googletest//:gtest_main",
        ],
    )
    for llvm_version in AVAILABLE_LLVM_VERSIONS
]
