load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

llvm_version = "18"

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
