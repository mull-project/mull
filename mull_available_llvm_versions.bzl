# buildifier: disable=module-docstring
load("@mull_package_info//:mull_package_info.bzl", "OS_NAME", "OS_VERSION")
load("@supported_llvm_versions//:supported_llvm_versions.bzl", "OS_VERSION_MAPPING")

def _is_macos(repository_ctx):
    return repository_ctx.os.name.find("mac") != -1

def _llvm_path(repository_ctx, version):
    if _is_macos(repository_ctx):
        return "/opt/homebrew/opt/llvm@" + version
    return "/usr/lib/llvm-" + version

def _is_supported(repository_ctx, version):
    path = _llvm_path(repository_ctx, version)
    return repository_ctx.path(path).exists

def _exact_version(repository_ctx, version):
    path = _llvm_path(repository_ctx, version)
    result = repository_ctx.execute([path + "/bin/llvm-config", "--version"])
    return result.stdout.strip()

def _clang_paths(repository_ctx, version):
    path = _llvm_path(repository_ctx, version)
    return [path + "/bin/clang", path + "/bin/clang++"]

CONTENT = """
AVAILABLE_LLVM_VERSIONS = {VERSIONS}
EXACT_VERSION_MAPPING = {MAPPING}
CC_PATHS = {CC_MAPPING}
CXX_PATHS = {CXX_MAPPING}
"""

def _llvm_versions_repo_impl(repository_ctx):
    available_versions = []
    mapping = {}
    cc_paths = {}
    cxx_paths = {}
    os_key = "%s:%s" % (OS_NAME, OS_VERSION)
    if OS_NAME.lower() == "macos":
        os_key = "macos"
    for version in OS_VERSION_MAPPING[os_key]:
        if _is_supported(repository_ctx, version):
            available_versions.append(version)
            mapping[version] = _exact_version(repository_ctx, version)
            compiler_paths = _clang_paths(repository_ctx, version)
            cc_paths[version] = compiler_paths[0]
            cxx_paths[version] = compiler_paths[1]
    if len(available_versions) == 0:
        fail("Could not find any supported LLVM versions installed")
    repository_ctx.file(
        "mull_llvm_versions.bzl",
        content = CONTENT.format(
            VERSIONS = str(available_versions),
            MAPPING = mapping,
            CC_MAPPING = cc_paths,
            CXX_MAPPING = cxx_paths,
        ),
    )
    repository_ctx.file(
        "BUILD",
        content = "",
    )

available_llvm_versions_repo = repository_rule(
    local = True,
    implementation = _llvm_versions_repo_impl,
)

def _available_llvm_versions_impl(_module_ctx):
    available_llvm_versions_repo(name = "available_llvm_versions")

available_llvm_versions = module_extension(
    implementation = _available_llvm_versions_impl,
    tag_classes = {
        "detect_available": tag_class(attrs = {}),
    },
)
