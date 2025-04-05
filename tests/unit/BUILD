load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(":fixtures.bzl", "fixture", "fixtures_header")

llvm_version = "18"

fixtures_header(
    name = "fixture_header",
    fixtures = [
        "//tests/unit/fixtures/simple_test/count_letters:test_count_letters.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c.bc",
        ":fixtures/hardcode/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator.ll",
    ],
    header = "FixturePaths.h",
)

cc_library(
    name = "FixturePaths",
    hdrs = [":FixturePaths.h"],
)

cc_library(
    name = "unit_test_helpers_%s" % llvm_version,
    srcs = [
        "TestModuleFactory.cpp",
    ] + glob(["helpers/*.cpp"]),
    hdrs = [
        "TestModuleFactory.h",
    ] + glob(["helpers/*.h"]),
    deps = [
        "//:libmull_%s" % llvm_version,
    ],
)

cc_test(
    name = "mull_unit_tests_%s" % llvm_version,
    size = "small",
    srcs = [
        "MutationFilters/GitDiffReaderTests.cpp",
        "MutationTestingElementsReporterTest.cpp",
        "Mutators/NegateConditionMutatorTest.cpp",
        "MutatorsFactoryTests.cpp",
        "TaskExecutorTests.cpp",
    ],
    data = [
        ":fixtures/hardcode/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator.ll",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:test_count_letters.c.bc",
    ],
    deps = [
        "//:libmull_%s" % llvm_version,
        ":unit_test_helpers_%s" % llvm_version,
        ":FixturePaths",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
