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
    name = "llvm_18",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":llvm_18_private"],
)

cc_import(
    name = "clang_18_private",
    hdrs = glob([
        "include/clang/**/*.h",
        "include/clang-c/**/*.h",
        "include/clang/**/*.def",
        "include/clang/**/*.inc",
    ]),
    shared_library = "lib/libclang.dylib",
)

cc_library(
    name = "clang_18",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":clang_18_private"],
)
