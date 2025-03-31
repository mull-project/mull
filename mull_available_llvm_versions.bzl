# buildifier: disable=module-docstring
UNSUPPORTED_MACOS_VERSIONS = ["12", "13", "14", "15"]
# clang-14 picks wrong -syslibroot for linking

def _is_macos(repository_ctx):
    return repository_ctx.os.name.find("mac") != -1

def _is_supported(repository_ctx, version):
    if _is_macos(repository_ctx):
        path = "/opt/homebrew/opt/llvm@" + version
        return (version not in UNSUPPORTED_MACOS_VERSIONS) and repository_ctx.path(path).exists
    path = "/usr/lib/llvm-" + version
    return repository_ctx.path(path).exists

def _llvm_versions_repo_impl(repository_ctx):
    available_versions = []
    for version in repository_ctx.attr.versions:
        if _is_supported(repository_ctx, version):
            available_versions.append(version)
    if len(available_versions) == 0:
        fail("Could not find any supported LLVM versions installed")
    repository_ctx.file(
        "mull_llvm_versions.bzl",
        content = "AVAILABLE_LLVM_VERSIONS = " + str(available_versions),
    )
    repository_ctx.file(
        "BUILD",
        content = "",
    )

available_llvm_versions_repo = repository_rule(
    local = True,
    implementation = _llvm_versions_repo_impl,
    attrs = {
        "versions": attr.string_list(),
    },
)

def _available_llvm_versions_impl(module_ctx):
    versions = []
    for mod in module_ctx.modules:
        for data in mod.tags.detect_available:
            for version in data.versions:
                versions.append(version)
    available_llvm_versions_repo(name = "available_llvm_versions", versions = versions)

available_llvm_versions = module_extension(
    implementation = _available_llvm_versions_impl,
    tag_classes = {
        "detect_available": tag_class(attrs = {"versions": attr.string_list(allow_empty = False)}),
    },
)
