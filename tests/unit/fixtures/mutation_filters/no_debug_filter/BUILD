load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

llvm_version = "18"

fixture(
    name = "with_debug.c.bc",
    args = [
        "-g",
        "-c",
        "-emit-llvm",
    ],
    compiler = "@llvm_%s//:clangxx" % llvm_version,
    fixture = "with_debug.c",
)

fixture(
    name = "without_debug.c.bc",
    args = [
        "-c",
        "-emit-llvm",
    ],
    compiler = "@llvm_%s//:clangxx" % llvm_version,
    fixture = "without_debug.c",
)
