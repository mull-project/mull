load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(":fixtures.bzl", "fixture", "fixtures_header")

llvm_version = "18"

fixtures_header(
    name = "fixture_header",
    fixtures = [
        "//tests/unit/fixtures/simple_test/count_letters:test_count_letters.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c.bc",
    ],
    header = "FixturePaths.h",
)

cc_library(
    name = "FixturePaths",
    hdrs = [":FixturePaths.h"],
)

cc_test(
    name = "mull_unit_tests_%s" % llvm_version,
    size = "small",
    srcs = [
        "MutationFilters/GitDiffReaderTests.cpp",
        "MutationTestingElementsReporterTest.cpp",
        "MutatorsFactoryTests.cpp",
        "TaskExecutorTests.cpp",
        "TestModuleFactory.cpp",
        "TestModuleFactory.h",
    ] + glob([
        "Helpers/*.cpp",
        "Helpers/*.h",
    ]),
    data = [
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:test_count_letters.c.bc",
    ],
    deps = [
        "//:libmull_%s" % llvm_version,
        ":FixturePaths",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

# [
#     cc_test(
#         name = "mull_unit_tests_%s" % llvm_version,
#         size = "small",
#         srcs = [
#             "MutationFilters/GitDiffReaderTests.cpp",
#             "MutatorsFactoryTests.cpp",
#             "TaskExecutorTests.cpp",
#         ],
#         deps = [
#             "//:libmull_%s" % llvm_version,
#             "@googletest//:gtest",
#             "@googletest//:gtest_main",
#         ],
#     )
#     for llvm_version in AVAILABLE_LLVM_VERSIONS
# ]
