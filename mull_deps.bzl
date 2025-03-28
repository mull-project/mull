# buildifier: disable=module-docstring
load("@bazel_skylib//lib:modules.bzl", "modules")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

supported_llvm_versions = tag_class(attrs = {"versions": attr.string_list()})

IRM_BUILD_FILE = """
load("@rules_cc//cc:defs.bzl", "cc_library")
cc_library(
    name = "irm",
    srcs = glob([
        "lib/**/*.cpp",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@llvm_{llvm_version}//:libllvm",
    ],
)
"""

def _mull_deps_extension(module_ctx):
    """Module extension to dynamically declare local LLVM repositories."""
    for mod in module_ctx.modules:
        for data in mod.tags.llvm:
            for version in data.versions:
                # Define each LLVM installation as a repository
                repo_name = "llvm_" + version
                path = "/opt/homebrew/opt/llvm@" + version
                compat = ""
                if not module_ctx.path(path).exists:
                    print("LLVM %s is not supported on this platform" % version)
                    compat = '"@platforms//:incompatible"'
                new_local_repository(
                    name = repo_name,
                    path = path,
                    build_file_content = LLVM_MACOS_BUILD_FILE.format(compat = compat),
                )
        for data in mod.tags.irm:
            for version in data.versions:
                http_archive(
                    name = "mull_irm_%s" % version,
                    integrity = "sha256-/UPOr/i+xfZEFv/oEVVIC0KhZxYRWVfzPdy7Ir7eNXU=",
                    urls = ["https://github.com/mull-project/libirm/archive/988a91877279c95bfe847994d006e3733a663962.zip"],
                    strip_prefix = "libirm-988a91877279c95bfe847994d006e3733a663962",
                    build_file_content = IRM_BUILD_FILE.format(llvm_version = version),
                )

    return modules.use_all_repos(module_ctx)

mull_deps = module_extension(
    implementation = _mull_deps_extension,
    tag_classes = {"llvm": supported_llvm_versions, "irm": supported_llvm_versions},
)

LLVM_MACOS_BUILD_FILE = """
load("@bazel_skylib//rules:native_binary.bzl", "native_binary")
load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

cc_import(
    name = "llvm_private",
    hdrs = glob([
        "include/llvm/**/*.h",
        "include/llvm-c/**/*.h",
        "include/llvm/**/*.def",
        "include/llvm/**/*.inc",
    ]),
    shared_library = "lib/libLLVM.dylib",
    target_compatible_with = [{compat}],
)

cc_library(
    name = "libllvm",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":llvm_private"],
    target_compatible_with = [{compat}],
)

cc_import(
    name = "libclang_private",
    hdrs = glob([
        "include/clang/**/*.h",
        "include/clang-c/**/*.h",
        "include/clang/**/*.def",
        "include/clang/**/*.inc",
    ]),
    shared_library = "lib/libclang-cpp.dylib",
    target_compatible_with = [{compat}],
)

cc_library(
    name = "libclang",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":libclang_private"],
    target_compatible_with = [{compat}],
)

native_binary(
    name = "clang",
    src = "bin/clang",
    out = "clang",
    visibility = ["//visibility:public"],
    target_compatible_with = [{compat}],
)

native_binary(
    name = "clangxx",
    src = "bin/clang++",
    out = "clangxx",
    visibility = ["//visibility:public"],
    target_compatible_with = [{compat}],
)

native_binary(
    name = "llvm-profdata",
    src = "bin/llvm-profdata",
    out = "llvm-profdata",
    visibility = ["//visibility:public"],
    target_compatible_with = [{compat}],
)
"""
