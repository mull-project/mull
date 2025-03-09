# buildifier: disable=module-docstring

CONTENT = """
OS_VERSION_MAPPING = {OS_VERSION_MAPPING}
"""

def _supported_llvm_versions_repo_impl(repository_ctx):
    repository_ctx.file(
        "supported_llvm_versions.bzl",
        content = CONTENT.format(
            OS_VERSION_MAPPING = repository_ctx.attr.os_version_mapping,
        ),
    )
    repository_ctx.file(
        "BUILD",
        content = "",
    )

supported_llvm_versions_repo = repository_rule(
    local = True,
    implementation = _supported_llvm_versions_repo_impl,
    attrs = {
        "os_version_mapping": attr.string_list_dict(),
    },
)

def _mull_supported_llvm_versions_impl(module_ctx):
    mapping = {}
    for mod in module_ctx.modules:
        for data in mod.tags.configure:
            mapping.update(data.os_version_mapping)
    supported_llvm_versions_repo(name = "supported_llvm_versions", os_version_mapping = mapping)

mull_supported_llvm_versions = module_extension(
    implementation = _mull_supported_llvm_versions_impl,
    tag_classes = {"configure": tag_class(attrs = {"os_version_mapping": attr.string_list_dict()})},
)
