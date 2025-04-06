# buildifier: disable=module-docstring

def _is_macos(repository_ctx):
    return repository_ctx.os.name.find("mac") != -1

def _os_release_kv(repository_ctx):
    lines = repository_ctx.read("/etc/os-release").strip().split("\n")
    kv = {}
    for line in lines:
        split = line.split("=", 1)
        k = split[0]
        v = split[1]
        kv[k] = v.strip().strip('"')
    return kv

def _os_name(repository_ctx):
    if _is_macos(repository_ctx):
        return "macOS"
    return _os_release_kv(repository_ctx)["ID"]

def _os_version(repository_ctx):
    if _is_macos(repository_ctx):
        result = repository_ctx.execute(["sw_vers", "--productVersion"])
        return result.stdout.strip()
    return _os_release_kv(repository_ctx)["VERSION_ID"]

def _mull_version(repository_ctx):
    version = repository_ctx.attr.mull_version
    extra_path = repository_ctx.path(str(repository_ctx.workspace_root) + "/.mull-ci-version-suffix")
    if extra_path.exists:
        content = repository_ctx.read(extra_path).strip()
        if not content:
            fail("empty version suffix file: %s" % extra_path)
        version += "-" + content
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
OS_ARCH = "{OS_ARCH}"
OS_VERSION = "{OS_VERSION}"
CS_REPO = "{CS_REPO}"
"""

def _mull_package_info_repo_impl(repository_ctx):
    repository_ctx.file(
        "mull_package_info.bzl",
        content = PACKAGE_INFO.format(
            OS_NAME = _os_name(repository_ctx),
            MULL_VERSION = _mull_version(repository_ctx),
            OS_ARCH = repository_ctx.os.arch,
            OS_VERSION = _os_version(repository_ctx),
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
