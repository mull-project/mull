load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")

# buildifier: disable=module-docstring
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

                # Define each LLVM installation as a repository
                path = "/opt/homebrew/opt/llvm@" + version
                new_local_repository(
                    name = llvm_repo_name,
                    path = path,
                    build_file = ":third_party/llvm/llvm.BUILD",
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
