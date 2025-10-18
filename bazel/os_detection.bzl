def _os_release_kv(repository_ctx):
    lines = repository_ctx.read("/etc/os-release").strip().split("\n")
    kv = {}
    for line in lines:
        split = line.split("=", 1)
        if len(split) != 2:
            continue
        k = split[0]
        v = split[1]
        kv[k] = v.strip().strip('"')
    return kv

def is_macos(repository_ctx):
    return repository_ctx.os.name.find("mac") != -1

def is_redhat(repository_ctx):
    if is_macos(repository_ctx):
        return False
    return _os_release_kv(repository_ctx)["ID"] == "rhel"

def os_name(repository_ctx):
    if is_macos(repository_ctx):
        return "macOS"
    return _os_release_kv(repository_ctx)["ID"]

def os_codename(repository_ctx):
    if is_macos(repository_ctx):
        return "macOS, don't care"
    if is_redhat(repository_ctx):
        return _os_release_kv(repository_ctx)["VERSION_ID"].split(".")[0]
    return _os_release_kv(repository_ctx)["VERSION_CODENAME"]

def os_version(repository_ctx):
    if is_macos(repository_ctx):
        result = repository_ctx.execute(["sw_vers", "--productVersion"])
        return result.stdout.strip()
    return _os_release_kv(repository_ctx)["VERSION_ID"]

def os_dist_extension(repository_ctx):
    if is_macos(repository_ctx):
        return "zip"
    if is_redhat(repository_ctx):
        return "rpm"
    return "deb"
