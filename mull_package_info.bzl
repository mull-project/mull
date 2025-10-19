# buildifier: disable=module-docstring

load("//:bazel/os_detection.bzl", "os_codename", "os_dist_extension", "os_name", "os_version")

def _mull_version(repository_ctx):
    version = repository_ctx.attr.mull_version
    extra_path = repository_ctx.path(str(repository_ctx.workspace_root) + "/.mull-ci-version-suffix")
    if extra_path.exists:
        content = repository_ctx.read(extra_path).strip()
        if not content:
            fail("empty version suffix file: %s" % extra_path)
        version += content
    return version

def _cs_repo(repository_ctx):
    cs_repo = repository_ctx.path(str(repository_ctx.workspace_root) + "/.mull-ci-cs-repo")
    if cs_repo.exists:
        content = repository_ctx.read(cs_repo).strip()
        if not content:
            fail("empty cloudsmith repo file: %s" % cs_repo)
        if content not in ["nightly", "stable", "testing"]:
            fail("unsupported cloudsmith repo '%s'" % content)
        return content
    return "testing"

PACKAGE_INFO = """
MULL_VERSION = "{MULL_VERSION}"
OS_NAME = "{OS_NAME}"
OS_CODENAME = "{OS_CODENAME}"
OS_ARCH = "{OS_ARCH}"
OS_VERSION = "{OS_VERSION}"
OS_DIST_EXTENSION = "{OS_DIST_EXTENSION}"
CS_REPO = "{CS_REPO}"
MULL_HOMEPAGE = "https://github.com/mull-project/mull"
MULL_DESCRIPTION = "Practical mutation testing and fault injection for C and C++"
MULL_SUPPORT_URL = "https://mull.readthedocs.io/en/latest/Support.html"
MULL_DOCS_URL = "https://mull.readthedocs.io"
"""

def _mull_package_info_repo_impl(repository_ctx):
    repository_ctx.file(
        "mull_package_info.bzl",
        content = PACKAGE_INFO.format(
            OS_NAME = os_name(repository_ctx),
            OS_CODENAME = os_codename(repository_ctx),
            OS_DIST_EXTENSION = os_dist_extension(repository_ctx),
            MULL_VERSION = _mull_version(repository_ctx),
            OS_ARCH = repository_ctx.os.arch,
            OS_VERSION = os_version(repository_ctx),
            CS_REPO = _cs_repo(repository_ctx),
        ),
    )
    repository_ctx.file(
        "BUILD",
        content = "",
    )

mull_package_info_repo = repository_rule(
    local = True,
    implementation = _mull_package_info_repo_impl,
    attrs = {
        "mull_version": attr.string(),
    },
)

def _mull_package_info_impl(module_ctx):
    version = ""
    for mod in module_ctx.modules:
        if mod.is_root:
            version = mod.version
    mull_package_info_repo(name = "mull_package_info", mull_version = version)

mull_package_info = module_extension(
    implementation = _mull_package_info_impl,
)
