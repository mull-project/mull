load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

[
    fixture(
        name = "%s_some_test_file_name.c.bc" % llvm_version,
        args = [
            "-c",
            "-g",
            "-emit-llvm",
        ],
        compiler = "@llvm_%s//:clang" % llvm_version,
        fixture = "some_test_file_name.c",
    )
    for llvm_version in AVAILABLE_LLVM_VERSIONS
]
