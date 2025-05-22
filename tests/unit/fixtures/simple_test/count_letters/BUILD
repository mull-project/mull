load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@bazel_itertools//lib:itertools.bzl", "itertools")
load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

[
    fixture(
        name = "%s_%s.bc" % (llvm_version, f),
        args = [
            "-g",
            "-c",
            "-emit-llvm",
        ],
        compiler = "@llvm_%s//:clang" % llvm_version,
        fixture = f,
    )
    for (f, llvm_version) in itertools.product(
        glob(["*.c"]),
        AVAILABLE_LLVM_VERSIONS,
    )
]
