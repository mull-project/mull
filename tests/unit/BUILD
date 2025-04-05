load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(":fixtures.bzl", "fixtures_header")

llvm_version = "18"

filegroup(
    name = "used_fixtures",
    srcs = [
        ":fixtures/hardcode/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator.ll",
        "//tests/unit/fixtures/junk_detection/compdb:compdb_absolute",
        "//tests/unit/fixtures/junk_detection/compdb:compdb_relative",
        "//tests/unit/fixtures/junk_detection/compdb:include",
        # "//tests/unit/fixtures/junk_detection/compdb:db_produced_from_clang_MJ_valid_sequence.json",
        # "//tests/unit/fixtures/junk_detection/compdb:db_with_arguments.json",
        # "//tests/unit/fixtures/junk_detection/compdb:db_with_commands.json",
        # "//tests/unit/fixtures/junk_detection/compdb:db_with_escaped_quotes.json",
        # "//tests/unit/fixtures/junk_detection/compdb:db_with_fullpath_compiler.json",
        # "//tests/unit/fixtures/junk_detection/compdb:db_with_output.json",
        "//tests/unit/fixtures/junk_detection/compdb:main.cpp.bc",
        "//tests/unit/fixtures/mutation_filters/file_path:some_test_file_name.c.bc",
        "//tests/unit/fixtures/mutation_filters/no_debug_filter:with_debug.c.bc",
        "//tests/unit/fixtures/mutation_filters/no_debug_filter:without_debug.c.bc",
        "//tests/unit/fixtures/mutators:and_or/and_to_or_replacement_cpp/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:and_or/or_to_and_replacement_cpp/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:bitwise/bitops.cpp.bc",
        "//tests/unit/fixtures/mutators:bitwise/shifts.cpp.bc",
        "//tests/unit/fixtures/mutators:boundary/module.cpp.bc",
        "//tests/unit/fixtures/mutators:math/bitwise_not.cpp.bc",
        "//tests/unit/fixtures/mutators:math/unary_minus.cpp.bc",
        "//tests/unit/fixtures/mutators:math_add/module.cpp.bc",
        "//tests/unit/fixtures/mutators:math_div/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:math_mul/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:math_sub/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:negate_condition/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:rem_to_div/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:remove_negation/main.c.bc",
        "//tests/unit/fixtures/mutators:remove_void_function/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:replace_assignment/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:replace_assignment/module.c.bc",
        "//tests/unit/fixtures/mutators:replace_call/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:replace_call/module.c.bc",
        "//tests/unit/fixtures/mutators:scalar_value/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:scalar_value/module.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:test_count_letters.c.bc",
    ],
)

fixtures_header(
    name = "fixture_header",
    fixtures = [":used_fixtures"],
    header = "FixturePaths.h",
)

cc_library(
    name = "FixturePaths",
    hdrs = [":FixturePaths.h"],
)

cc_library(
    name = "unit_test_helpers_%s" % llvm_version,
    srcs = glob(["helpers/*.cpp"]),
    hdrs = glob(["helpers/*.h"]),
    deps = ["//:libmull_%s" % llvm_version],
)

filegroup(
    name = "MutationFilters/GitDiffReaderTests.cpp_%s_fixtures" % llvm_version,
)

filegroup(
    name = "MutationFilters/MutationFilterTests.cpp_%s_fixtures" % llvm_version,
    srcs = [
        "//tests/unit/fixtures/mutation_filters/file_path:some_test_file_name.c.bc",
        "//tests/unit/fixtures/mutation_filters/no_debug_filter:with_debug.c.bc",
        "//tests/unit/fixtures/mutation_filters/no_debug_filter:without_debug.c.bc",
    ],
)

filegroup(
    name = "Mutators/ConditionalsBoundaryMutatorTests.cpp_%s_fixtures" % llvm_version,
    srcs = ["//tests/unit/fixtures/mutators:boundary/module.cpp.bc"],
)

filegroup(
    name = "Mutators/NegateConditionMutatorTest.cpp_%s_fixtures" % llvm_version,
    srcs = [":fixtures/hardcode/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator.ll"],
)

filegroup(
    name = "Mutators/ScalarValueMutatorTest.cpp_%s_fixtures" % llvm_version,
    srcs = ["//tests/unit/fixtures/mutators:scalar_value/module.c.bc"],
)

filegroup(
    name = "MutationTestingElementsReporterTest.cpp_%s_fixtures" % llvm_version,
    srcs = [
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c",
        "//tests/unit/fixtures/simple_test/count_letters:count_letters.c.bc",
        "//tests/unit/fixtures/simple_test/count_letters:test_count_letters.c.bc",
    ],
)

filegroup(
    name = "MutatorsFactoryTests.cpp_%s_fixtures" % llvm_version,
)

filegroup(
    name = "TaskExecutorTests.cpp_%s_fixtures" % llvm_version,
)

filegroup(
    name = "MutationPointTests.cpp_%s_fixtures" % llvm_version,
    srcs = [
        "//tests/unit/fixtures/mutators:replace_assignment/module.c",
        "//tests/unit/fixtures/mutators:replace_assignment/module.c.bc",
        "//tests/unit/fixtures/mutators:replace_call/module.c.bc",
        "//tests/unit/fixtures/mutators:scalar_value/module.c.bc",
    ],
)

filegroup(
    name = "JunkDetection/CXXJunkDetectorTests.cpp_%s_fixtures" % llvm_version,
    srcs = [
        "//tests/unit/fixtures/junk_detection/compdb:compdb_absolute",
        "//tests/unit/fixtures/junk_detection/compdb:compdb_relative",
        "//tests/unit/fixtures/junk_detection/compdb:include/header.h",
        "//tests/unit/fixtures/junk_detection/compdb:main.cpp",
        "//tests/unit/fixtures/junk_detection/compdb:main.cpp.bc",
        "//tests/unit/fixtures/mutators:and_or/and_to_or_replacement_cpp/junk.cpp",
        "//tests/unit/fixtures/mutators:and_or/and_to_or_replacement_cpp/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:and_or/or_to_and_replacement_cpp/junk.cpp",
        "//tests/unit/fixtures/mutators:and_or/or_to_and_replacement_cpp/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:bitwise/bitops.cpp",
        "//tests/unit/fixtures/mutators:bitwise/bitops.cpp.bc",
        "//tests/unit/fixtures/mutators:bitwise/shifts.cpp",
        "//tests/unit/fixtures/mutators:bitwise/shifts.cpp.bc",
        "//tests/unit/fixtures/mutators:boundary/module.cpp",
        "//tests/unit/fixtures/mutators:boundary/module.cpp.bc",
        "//tests/unit/fixtures/mutators:math/bitwise_not.cpp",
        "//tests/unit/fixtures/mutators:math/bitwise_not.cpp.bc",
        "//tests/unit/fixtures/mutators:math/unary_minus.cpp",
        "//tests/unit/fixtures/mutators:math/unary_minus.cpp.bc",
        "//tests/unit/fixtures/mutators:math_add/module.cpp",
        "//tests/unit/fixtures/mutators:math_add/module.cpp.bc",
        "//tests/unit/fixtures/mutators:math_div/junk.cpp",
        "//tests/unit/fixtures/mutators:math_div/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:math_mul/junk.cpp",
        "//tests/unit/fixtures/mutators:math_mul/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:math_sub/junk.cpp",
        "//tests/unit/fixtures/mutators:math_sub/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:negate_condition/junk.cpp",
        "//tests/unit/fixtures/mutators:negate_condition/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:rem_to_div/junk.cpp",
        "//tests/unit/fixtures/mutators:rem_to_div/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:remove_negation/main.c",
        "//tests/unit/fixtures/mutators:remove_negation/main.c.bc",
        "//tests/unit/fixtures/mutators:remove_void_function/junk.cpp",
        "//tests/unit/fixtures/mutators:remove_void_function/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:replace_assignment/junk.cpp",
        "//tests/unit/fixtures/mutators:replace_assignment/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:replace_call/junk.cpp",
        "//tests/unit/fixtures/mutators:replace_call/junk.cpp.bc",
        "//tests/unit/fixtures/mutators:scalar_value/junk.cpp",
        "//tests/unit/fixtures/mutators:scalar_value/junk.cpp.bc",
    ],
)

cc_test(
    name = "JunkDetection/CXXJunkDetectorTests.cpp_%s" % llvm_version,
    size = "small",
    srcs = ["JunkDetection/CXXJunkDetectorTests.cpp"],
    data = [
        "JunkDetection/CXXJunkDetectorTests.cpp_%s_fixtures" % llvm_version,
    ],
    deps = [
        "//:libmull_%s" % llvm_version,
        ":unit_test_helpers_%s" % llvm_version,
        ":FixturePaths",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

[
    cc_test(
        name = "%s_%s" % (f, llvm_version),
        size = "small",
        srcs = [f],
        data = [
            "%s_%s_fixtures" % (f, llvm_version),
        ],
        deps = [
            "//:libmull_%s" % llvm_version,
            ":unit_test_helpers_%s" % llvm_version,
            ":FixturePaths",
            "@googletest//:gtest",
            "@googletest//:gtest_main",
        ],
    )
    for f in glob([
        "MutationFilters/**/*.cpp",
        "Mutators/**/*.cpp",
        "*.cpp",
    ])
]

[
    cc_test(
        name = "%s_%s" % (f, llvm_version),
        size = "small",
        srcs = [f],
        deps = [
            "//:libmull_%s" % llvm_version,
            ":unit_test_helpers_%s" % llvm_version,
            "@googletest//:gtest",
            "@googletest//:gtest_main",
        ],
    )
    for f in glob(["Mutations-E2E/**/*.cpp"])
]
