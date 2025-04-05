load("@bazel_skylib//rules:expand_template.bzl", "expand_template")
load("//tests/unit:fixtures.bzl", "fixture")

package(default_visibility = ["//visibility:public"])

exports_files(glob(["*.json"]) + ["include"])

llvm_version = "18"

fixture(
    name = "%s_main.cpp.bc" % llvm_version,
    args = [
        "-g",
        "-c",
        "-emit-llvm",
        "-Iinclude",
    ],
    compiler = "@llvm_%s//:clangxx" % llvm_version,
    data = [
        ":include/header.h",
    ],
    fixture = "main.cpp",
)

expand_template(
    name = "compdb_absolute",
    out = "absolute_compile_commands.json",
    substitutions = {
        "@DIR@": package_name(),
        "@INC@": package_name(),
    },
    template = ":compile_commands.json.in",
)

expand_template(
    name = "compdb_relative",
    out = "relative_compile_commands.json",
    substitutions = {
        "@DIR@": package_name(),
        "@INC@": ".",
    },
    template = ":compile_commands.json.in",
)
