# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@bazel_skylib//lib:modules.bzl", "modules")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")
load("//:bazel/os_detection.bzl", "is_macos", "is_redhat")

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
        "@llvm_{LLVM_VERSION}//:libllvm",
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
    shared_library = "{LIBDIR}/{LIBLLVM_DYLIB}",
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
    shared_library = "{LIBDIR}/{LIBCLANG_CPP_DYLIB}",
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

def _empty_repo_impl(repository_ctx):
    repository_ctx.file(
        "BUILD",
        content = "",
    )

empty_repo = repository_rule(
    local = True,
    implementation = _empty_repo_impl,
)

def _dylib_ext(ctx):
    if is_macos(ctx):
        return "dylib"
    return "so"

def _find_dylib(ctx, path, lib, dylib, version):
    libdir = path + "/" + lib
    vdylib = "lib" + dylib + "-" + version + "." + _dylib_ext(ctx)
    dylib = "lib" + dylib + "." + _dylib_ext(ctx)
    for f in ctx.path(libdir).readdir():
        if f.basename.startswith(dylib):
            return f.basename
        if f.basename.startswith(vdylib):
            return f.basename

    return None

def _find_clang_dylib(ctx, path, lib, version):
    return _find_dylib(ctx, path, lib, "clang-cpp", version)

def _find_llvm_dylib(ctx, path, lib, version):
    return _find_dylib(ctx, path, lib, "LLVM", version)

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

                libdir = "lib"
                if is_macos(module_ctx):
                    path = "/opt/homebrew/opt/llvm@" + version
                elif is_redhat(module_ctx):
                    path = "/usr"
                    libdir = "lib64"
                else:
                    path = "/usr/lib/llvm-" + version
                llvm_dylib = _find_llvm_dylib(module_ctx, path, libdir, version)
                clang_dylib = _find_clang_dylib(module_ctx, path, libdir, version)
                new_local_repository(
                    name = llvm_repo_name,
                    path = path,
                    build_file_content = LLVM_BUILD_FILE.format(
                        LIBLLVM_DYLIB = llvm_dylib,
                        LIBCLANG_CPP_DYLIB = clang_dylib,
                        LLVM_VERSION = version,
                        LIBDIR = libdir,
                    ),
                )
                http_archive(
                    name = irm_repo_name,
                    integrity = "sha256-8pmIPDJX0cgDlNljcIcWd73Wb2WB8cgK/086RxOyqrE=",
                    urls = ["https://github.com/mull-project/libirm/archive/08eab0634575aeb721d07f05daf4a0aad8feba36.zip"],
                    strip_prefix = "libirm-08eab0634575aeb721d07f05daf4a0aad8feba36",
                    build_file_content = IRM_BUILD_FILE.format(LLVM_VERSION = version),
                )

    return modules.use_all_repos(module_ctx)

mull_deps = module_extension(
    implementation = _mull_deps_extension,
    tag_classes = {"configure": tag_class(attrs = {"versions": attr.string_list()})},
)
