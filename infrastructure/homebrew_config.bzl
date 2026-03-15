# Generate Homebrew config from Bazel

load("@mull_package_info//:mull_package_info.bzl", "MULL_VERSION")
load("@supported_llvm_versions//:supported_llvm_versions.bzl", "OS_VERSION_MAPPING")

def _homebrew_config_impl(ctx):
    llvm_versions = OS_VERSION_MAPPING.get("macos", [])

    out = ctx.actions.declare_file("homebrew_config.json")
    content = '''{{
  "mull_version": "{version}",
  "llvm_versions": [{llvm_versions}]
}}
'''.format(
        version = MULL_VERSION,
        llvm_versions = ", ".join(llvm_versions),
    )
    ctx.actions.write(out, content)
    return [DefaultInfo(files = depset([out]))]

homebrew_config = rule(
    implementation = _homebrew_config_impl,
)
