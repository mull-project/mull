load("@bazel_skylib//lib:modules.bzl", "modules")
load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

def _detect_llvm_versions():
    """Detects available LLVM versions and returns a list of installed versions."""
    llvm_versions = ["17", "18"]

    # for version in ["17", "18"]:
    #     llvm_path = which("llvm-config-" + version)
    #     if llvm_path:
    #         llvm_versions.append(version)
    return llvm_versions

def _llvm_module_extension(module_ctx):
    """Module extension to dynamically declare local LLVM repositories."""
    llvm_versions = _detect_llvm_versions()

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
