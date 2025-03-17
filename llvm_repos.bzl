load("@bazel_skylib//lib:modules.bzl", "modules")
load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

def _llvm_module_extension(module_ctx):
    """Module extension to dynamically declare local LLVM repositories."""
    llvm_versions = [
        "12",
        "13",
        "14",
        "15",
        "16",
        "17",
        "18",
    ]

    # Define each LLVM installation as a repository
    for version in llvm_versions:
        repo_name = "llvm_" + version
        path = "/opt/homebrew/opt/llvm@" + version
        new_local_repository(
            name = repo_name,
            path = path,
            build_file = ":third_party/llvm/llvm.BUILD",
        )
    return modules.use_all_repos(module_ctx)

llvm_repos = module_extension(
    implementation = _llvm_module_extension,
)
