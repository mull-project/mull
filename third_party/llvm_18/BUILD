load("@bazel_skylib//rules:native_binary.bzl", "native_binary")
load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

cc_import(
    name = "llvm_18_private",
    hdrs = glob([
        "include/llvm/**/*.h",
        "include/llvm-c/**/*.h",
        "include/llvm/**/*.def",
        "include/llvm/**/*.inc",
    ]),
    shared_library = "lib/libLLVM.dylib",
)

cc_library(
    name = "libllvm_18",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":llvm_18_private"],
)

cc_import(
    name = "libclang_18_private",
    hdrs = glob([
        "include/clang/**/*.h",
        "include/clang-c/**/*.h",
        "include/clang/**/*.def",
        "include/clang/**/*.inc",
    ]),
    shared_library = "lib/libclang-cpp.dylib",
)

cc_library(
    name = "libclang_18",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":libclang_18_private"],
)

native_binary(
    name = "clang",
    src = "bin/clang",
    out = "clang",
    visibility = ["//visibility:public"],
)

native_binary(
    name = "clangxx",
    src = "bin/clang++",
    out = "clangxx",
    visibility = ["//visibility:public"],
)

native_binary(
    name = "llvm-profdata",
    src = "bin/llvm-profdata",
    out = "llvm-profdata",
    visibility = ["//visibility:public"],
)
