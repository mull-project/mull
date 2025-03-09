load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

[
    fixture(
        name = "%s_with_debug.c.bc" % llvm_version,
        args = [
            "-g",
            "-c",
            "-emit-llvm",
        ],
        compiler = "@llvm_%s//:clang" % llvm_version,
        fixture = "with_debug.c",
    )
    for llvm_version in AVAILABLE_LLVM_VERSIONS
]

[
    fixture(
        name = "%s_without_debug.c.bc" % llvm_version,
        args = [
            "-c",
            "-emit-llvm",
        ],
        compiler = "@llvm_%s//:clang" % llvm_version,
        fixture = "without_debug.c",
    )
    for llvm_version in AVAILABLE_LLVM_VERSIONS
]
