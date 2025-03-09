"Generate devcontainer files"

load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_files")
load("@bazel_skylib//rules:run_binary.bzl", "run_binary")
load("@rules_multirun//:defs.bzl", "multirun")
load("@supported_llvm_versions//:supported_llvm_versions.bzl", "OS_VERSION_MAPPING")

def _generate_github_workflow(runner, os_name, os_key):
    if os_key == "macos":
        version = "latest"
        name = os_name
        template = "macos"
    else:
        version = os_key.split(":")[-1]
        name = os_name + "_" + version
        template = "ubuntu"

    run_binary(
        name = "gh_workflow_%s" % name,
        srcs = [":templates/github-actions/ci-%s.yml.mustache" % template],
        outs = ["workflow_%s.yml" % name],
        args = [
            "--template",
            "infrastructure/templates/github-actions/ci-%s.yml.mustache" % template,
            "--output",
            "$(location workflow_%s.yml)" % name,
            "--runner",
            runner,
            "--os-name",
            os_name,
            "--os-version",
            version,
            "--llvm-versions",
        ] + OS_VERSION_MAPPING[os_key],
        tool = ":gh_workflow_gen",
    )

    target_name = "write_gh_workflow_%s_files" % name
    write_source_files(
        name = target_name,
        files = {
            "//.github/workflows:ci-%s.yml" % name: "workflow_%s.yml" % name,
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
            write_targets.append(_generate_github_workflow("macos-latest", "macOS", os_key))
        else:
            write_targets.append(_generate_github_workflow("ubuntu-latest", "Ubuntu", os_key))
            write_targets.append(_generate_github_workflow("ubuntu-24.04-arm", "Ubuntu-arm64", os_key))
    multirun(
        name = name,
        commands = write_targets,
        jobs = 0,
    )
