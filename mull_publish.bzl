load("@mull_package_info//:mull_package_info.bzl", "CS_REPO", "MULL_VERSION", "OS_NAME")

MACOS_SCRIPT = """
./cloudsmith_runner push raw \\
  --name mull-{LLVM_VERSION} \\
  --version {MULL_VERSION} \\
  mull-project/mull-{CS_REPO} \\
  --tags macos \\
  {PACKAGE_FILE}
"""

PRE_SCRIPT = """
"""

def _publish_impl(ctx):
    cmds = []
    cmds.append(PRE_SCRIPT)
    for (idx, llvm_version) in enumerate(ctx.attr.packages):
        cmd = MACOS_SCRIPT.format(
            LLVM_VERSION = llvm_version,
            MULL_VERSION = MULL_VERSION,
            CS_REPO = CS_REPO,
            PACKAGE_FILE = ctx.files.packages[idx].short_path,
        )
        cmds.append(cmd)

    script = ctx.actions.declare_file("publish.sh")
    ctx.actions.write(script, "".join(cmds), is_executable = True)

    return [DefaultInfo(files = depset([script]))]

mull_publish_script = rule(
    implementation = _publish_impl,
    attrs = {
        "packages": attr.string_keyed_label_dict(allow_files = True),
    },
)
