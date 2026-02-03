load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(":fixtures.bzl", "fixtures_header")

def mull_unit_tests(name):
    native.filegroup(
        name = "fixtures",
        srcs = [
            ":fixtures/hardcode/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_negate_mutator.ll",
            "//tests/unit/fixtures/junk_detection/compdb:compdb_absolute",
            "//tests/unit/fixtures/junk_detection/compdb:compdb_relative",
            "//tests/unit/fixtures/junk_detection/compdb:db_produced_from_clang_MJ_valid_sequence.json",
            "//tests/unit/fixtures/junk_detection/compdb:db_with_arguments.json",
            "//tests/unit/fixtures/junk_detection/compdb:db_with_commands.json",
            "//tests/unit/fixtures/junk_detection/compdb:db_with_escaped_quotes.json",
            "//tests/unit/fixtures/junk_detection/compdb:db_with_fullpath_compiler.json",
            "//tests/unit/fixtures/junk_detection/compdb:db_with_output.json",
            "//tests/unit/fixtures/junk_detection/compdb:include",
        ],
    )
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        native.filegroup(
            name = "generated_fixtures_%s" % llvm_version,
            srcs = [
                "//tests/unit/fixtures/junk_detection/compdb:%s_main.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutation_filters/file_path:%s_some_test_file_name.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutation_filters/no_debug_filter:%s_with_debug.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutation_filters/no_debug_filter:%s_without_debug.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_bitwise/bitops.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_bitwise/shifts.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_boundary/module.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_math/bitwise_not.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_math/unary_minus.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_math_add/module.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_math_div/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_math_mul/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_math_sub/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_negate_condition/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_rem_to_div/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_remove_negation/main.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_remove_void_function/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_replace_assignment/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_replace_assignment/module.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_replace_call/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_replace_call/module.c.bc" % llvm_version,
                "//tests/unit/fixtures/simple_test/count_letters:%s_count_letters.c.bc" % llvm_version,
                "//tests/unit/fixtures/simple_test/count_letters:%s_test_count_letters.c.bc" % llvm_version,
            ],
        )

        fixtures_header(
            name = "fixture_header_%s" % llvm_version,
            fixtures = [
                "generated_fixtures_%s" % llvm_version,
                ":fixtures",
            ],
            header = "%s/FixturePaths.h" % llvm_version,
            llvm_version = llvm_version,
        )

        cc_library(
            name = "fixture_header_lib_%s" % llvm_version,
            hdrs = [
                "fixture_header_%s" % llvm_version,
            ],
            strip_include_prefix = llvm_version,
            tags = ["unit", "llvm_%s" % llvm_version],
        )

        cc_library(
            name = "unit_test_helpers_%s" % llvm_version,
            srcs = native.glob(["Helpers/*.cpp"]),
            hdrs = native.glob(["Helpers/*.h"]),
            deps = ["//:libmull_%s" % llvm_version],
            tags = ["unit", "llvm_%s" % llvm_version],
        )

        native.filegroup(
            name = "MutationFilters/GitDiffReaderTests.cpp_%s_fixtures" % llvm_version,
        )

        native.filegroup(
            name = "MutationFilters/MutationFilterTests.cpp_%s_fixtures" % llvm_version,
            srcs = [
                "//tests/unit/fixtures/mutation_filters/file_path:%s_some_test_file_name.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutation_filters/no_debug_filter:%s_with_debug.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutation_filters/no_debug_filter:%s_without_debug.c.bc" % llvm_version,
            ],
        )

        native.filegroup(
            name = "Mutators/ConditionalsBoundaryMutatorTests.cpp_%s_fixtures" % llvm_version,
            srcs = ["//tests/unit/fixtures/mutators:%s_boundary/module.cpp.bc" % llvm_version],
        )

        native.filegroup(
            name = "Mutators/NegateConditionMutatorTest.cpp_%s_fixtures" % llvm_version,
            srcs = [":fixtures/hardcode/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_negate_mutator.ll"],
        )

        native.filegroup(
            name = "MutationTestingElementsReporterTest.cpp_%s_fixtures" % llvm_version,
            srcs = [
                "//tests/unit/fixtures/simple_test/count_letters:count_letters.c",
                "//tests/unit/fixtures/simple_test/count_letters:%s_count_letters.c.bc" % llvm_version,
                "//tests/unit/fixtures/simple_test/count_letters:%s_test_count_letters.c.bc" % llvm_version,
            ],
        )

        native.filegroup(
            name = "MutatorsFactoryTests.cpp_%s_fixtures" % llvm_version,
        )

        native.filegroup(
            name = "TaskExecutorTests.cpp_%s_fixtures" % llvm_version,
        )

        native.filegroup(
            name = "MutationPointTests.cpp_%s_fixtures" % llvm_version,
            srcs = [
                "//tests/unit/fixtures/mutators:replace_assignment/module.c",
                "//tests/unit/fixtures/mutators:%s_replace_assignment/module.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_replace_call/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:%s_replace_call/module.c.bc" % llvm_version,
            ],
        )

        native.filegroup(
            name = "JunkDetection/CXXJunkDetectorTests.cpp_%s_fixtures" % llvm_version,
            srcs = [
                "//tests/unit/fixtures/junk_detection/compdb:compdb_absolute",
                "//tests/unit/fixtures/junk_detection/compdb:compdb_relative",
                "//tests/unit/fixtures/junk_detection/compdb:include/header.h",
                "//tests/unit/fixtures/junk_detection/compdb:main.cpp",
                "//tests/unit/fixtures/junk_detection/compdb:%s_main.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:bitwise/bitops.cpp",
                "//tests/unit/fixtures/mutators:%s_bitwise/bitops.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:bitwise/shifts.cpp",
                "//tests/unit/fixtures/mutators:%s_bitwise/shifts.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:boundary/module.cpp",
                "//tests/unit/fixtures/mutators:%s_boundary/module.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:math/bitwise_not.cpp",
                "//tests/unit/fixtures/mutators:%s_math/bitwise_not.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:math/unary_minus.cpp",
                "//tests/unit/fixtures/mutators:%s_math/unary_minus.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:math_add/module.cpp",
                "//tests/unit/fixtures/mutators:%s_math_add/module.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:math_div/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_math_div/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:math_mul/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_math_mul/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:math_sub/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_math_sub/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:negate_condition/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_negate_condition/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:rem_to_div/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_rem_to_div/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:remove_negation/main.c",
                "//tests/unit/fixtures/mutators:%s_remove_negation/main.c.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:remove_void_function/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_remove_void_function/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:replace_assignment/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_replace_assignment/junk.cpp.bc" % llvm_version,
                "//tests/unit/fixtures/mutators:replace_call/junk.cpp",
                "//tests/unit/fixtures/mutators:%s_replace_call/junk.cpp.bc" % llvm_version,
            ],
        )

        native.filegroup(
            name = "JunkDetection/CompilationDatabaseTests.cpp_%s_fixtures" % llvm_version,
            srcs = [
                "//tests/unit/fixtures/junk_detection/compdb:db_produced_from_clang_MJ_valid_sequence.json",
                "//tests/unit/fixtures/junk_detection/compdb:db_with_arguments.json",
                "//tests/unit/fixtures/junk_detection/compdb:db_with_commands.json",
                "//tests/unit/fixtures/junk_detection/compdb:db_with_escaped_quotes.json",
                "//tests/unit/fixtures/junk_detection/compdb:db_with_fullpath_compiler.json",
                "//tests/unit/fixtures/junk_detection/compdb:db_with_output.json",
            ],
        )

        native.filegroup(name = "Mutations-E2E/CXX/Arithmetic/Mutation_Arithmetic_Add_Test.cpp_%s_fixtures" % llvm_version)

        native.filegroup(name = "Mutations-E2E/CXX/Arithmetic/Mutation_Arithmetic_UnaryMinusToNoop_Test.cpp_%s_fixtures" % llvm_version)

        native.filegroup(name = "Mutations-E2E/CXX/ConstAssignment/Mutation_ConstAssignment_AssignConst.cpp_%s_fixtures" % llvm_version)

        native.filegroup(name = "Mutations-E2E/CXX/ConstAssignment/Mutation_ConstAssignment_InitConst.cpp_%s_fixtures" % llvm_version)

        native.filegroup(name = "Mutations-E2E/CXX/RemoveNegationTest.cpp_%s_fixtures" % llvm_version)

        native.filegroup(name = "Mutations-E2E/Scalar/01_Mutation_Scalar_ReturnValue_Test.cpp_%s_fixtures" % llvm_version)

        native.filegroup(name = "Mutations-E2E/Scalar/02_Mutation_Scalar_BinaryOperand_Test.cpp_%s_fixtures" % llvm_version)

        for f in native.glob([
            "MutationFilters/**/*.cpp",
            "Mutators/**/*.cpp",
            "JunkDetection/**/*.cpp",
            "*.cpp",
            "Mutations-E2E/**/*.cpp",
        ]):
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
                    ":fixture_header_lib_%s" % llvm_version,
                    "@googletest//:gtest",
                    "@googletest//:gtest_main",
                ],
                tags = ["unit", "llvm_%s" % llvm_version],
            )
