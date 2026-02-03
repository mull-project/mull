"Generate devcontainer files"

load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_files")
load("@bazel_skylib//rules:run_binary.bzl", "run_binary")
load("@rules_multirun//:defs.bzl", "multirun")
load("@supported_llvm_versions//:supported_llvm_versions.bzl", "OS_VERSION_MAPPING")

def _generate_github_workflow(runner, os_name, os_key, ext, arch):
    if os_key == "macos":
        version = "latest"
        name = os_name
        template = "macos"
    else:
        version = os_key.split(":")[1]
        name = os_name + "_" + version
        template = "linux"

    output_name = name + "-" + arch
    run_binary(
        name = "gh_workflow_%s" % output_name,
        srcs = [":templates/github-actions/ci-%s.yml.mustache" % template],
        outs = ["workflow_%s.yml" % output_name],
        args = [
            "--template",
            "infrastructure/templates/github-actions/ci-%s.yml.mustache" % template,
            "--output",
            "$(location workflow_%s.yml)" % output_name,
            "--runner",
            runner,
            "--os-name",
            os_name,
            "--os-version",
            version,
            "--package-extension",
            ext,
            "--arch",
            arch,
            "--llvm-versions",
        ] + OS_VERSION_MAPPING[os_key],
        tool = ":gh_workflow_gen",
    )

    target_name = "generate_gh_workflow_%s_files" % output_name
    write_source_files(
        name = target_name,
        files = {
            "//.github/workflows:ci-%s.yml" % output_name: "workflow_%s.yml" % output_name,
        },
        check_that_out_file_exists = False,
        suggested_update_target = name,
    )
    return target_name

def mull_github_workflows(name):
    """Defines targets to generate GitHub Actions workflow files

    Args:
      name: common target which generates all the files
    """
    write_targets = []
    for os_key in OS_VERSION_MAPPING:
        if os_key == "macos":
            write_targets.append(_generate_github_workflow("macos-latest", "macOS", os_key, "zip", "arm64"))
        elif "rhel" in os_key:
            write_targets.append(_generate_github_workflow("ubuntu-latest", "RHEL", os_key, "rpm", "x86_64"))
            write_targets.append(_generate_github_workflow("ubuntu-24.04-arm", "RHEL", os_key, "rpm", "arm64"))
        elif "debian" in os_key:
            write_targets.append(_generate_github_workflow("ubuntu-latest", "Debian", os_key, "deb", "x86_64"))
            write_targets.append(_generate_github_workflow("ubuntu-24.04-arm", "Debian", os_key, "deb", "arm64"))
        else:
            write_targets.append(_generate_github_workflow("ubuntu-latest", "Ubuntu", os_key, "deb", "x86_64"))
            write_targets.append(_generate_github_workflow("ubuntu-24.04-arm", "Ubuntu", os_key, "deb", "arm64"))
    multirun(
        name = name,
        commands = write_targets,
        jobs = 0,
    )
