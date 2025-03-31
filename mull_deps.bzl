# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@bazel_skylib//lib:modules.bzl", "modules")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

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

LLVM_BUILD_FILE = """
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
    shared_library = "lib/libLLVM.{LIBLLVM_EXT}",
)

cc_library(
    name = "libllvm",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":llvm_private"],
)

cc_import(
    name = "libclang_private",
    hdrs = glob([
        "include/clang/**/*.h",
        "include/clang-c/**/*.h",
        "include/clang/**/*.def",
        "include/clang/**/*.inc",
    ]),
    shared_library = "lib/libclang-cpp.{LIBCLANG_EXT}",
)

cc_library(
    name = "libclang",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":libclang_private"],
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
"""

def _is_macos(repository_ctx):
    return repository_ctx.os.name.find("mac") != -1

def _libllvm_ext(repository_ctx, _version):
    if _is_macos(repository_ctx):
        return "dylib"
    return "so"

def _libclang_ext(repository_ctx, version):
    if _is_macos(repository_ctx):
        return "dylib"
    return "so." + version

def _empty_repo_impl(repository_ctx):
    repository_ctx.file(
        "BUILD",
        content = "",
    )

empty_repo = repository_rule(
    local = True,
    implementation = _empty_repo_impl,
)

def _mull_deps_extension(module_ctx):
    """Module extension to dynamically declare local LLVM repositories."""
    for mod in module_ctx.modules:
        for data in mod.tags.configure:
            for version in data.versions:
                llvm_repo_name = "llvm_" + version
                irm_repo_name = "mull_irm_" + version
                if version not in AVAILABLE_LLVM_VERSIONS:
                    empty_repo(name = llvm_repo_name)
                    empty_repo(name = irm_repo_name)
                    continue

                if _is_macos(module_ctx):
                    path = "/opt/homebrew/opt/llvm@" + version
                else:
                    path = "/usr/lib/llvm-" + version
                new_local_repository(
                    name = llvm_repo_name,
                    path = path,
                    build_file_content = LLVM_BUILD_FILE.format(
                        LIBLLVM_EXT = _libllvm_ext(module_ctx, version),
                        LIBCLANG_EXT = _libclang_ext(module_ctx, version),
                    ),
                )
                http_archive(
                    name = irm_repo_name,
                    integrity = "sha256-/UPOr/i+xfZEFv/oEVVIC0KhZxYRWVfzPdy7Ir7eNXU=",
                    urls = ["https://github.com/mull-project/libirm/archive/988a91877279c95bfe847994d006e3733a663962.zip"],
                    strip_prefix = "libirm-988a91877279c95bfe847994d006e3733a663962",
                    build_file_content = IRM_BUILD_FILE.format(llvm_version = version),
                )

    return modules.use_all_repos(module_ctx)

mull_deps = module_extension(
    implementation = _mull_deps_extension,
    tag_classes = {"configure": tag_class(attrs = {"versions": attr.string_list()})},
)
