load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

llvm_version = "18"

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

fixture(
    name = "%s_without_debug.c.bc" % llvm_version,
    args = [
        "-c",
        "-emit-llvm",
    ],
    compiler = "@llvm_%s//:clang" % llvm_version,
    fixture = "without_debug.c",
)
