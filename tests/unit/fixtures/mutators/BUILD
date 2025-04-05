load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

llvm_version = "18"

[
    fixture(
        name = "%s.bc" % f,
        args = [
            "-g",
            "-c",
            "-emit-llvm",
        ],
        compiler = "@llvm_%s//:clangxx" % llvm_version,
        fixture = f,
    )
    for f in glob([
        "**/*.cpp",
    ])
]

[
    fixture(
        name = "%s.bc" % f,
        args = [
            "-g",
            "-c",
            "-emit-llvm",
        ],
        compiler = "@llvm_%s//:clang" % llvm_version,
        fixture = f,
    )
    for f in glob(["**/*.c"])
]
