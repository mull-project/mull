"Generate devcontainer files"

load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_files")
load("@bazel_skylib//rules:run_binary.bzl", "run_binary")
load("@rules_multirun//:defs.bzl", "multirun")
load("@supported_llvm_versions//:supported_llvm_versions.bzl", "OS_VERSION_MAPPING")

def _remap_os_key(os_key):
    if "rhel" in os_key:
        version = os_key.split(":")[1]
        major = version.split(".")[0]
        return "redhat/ubi%s:%s" % (major, version)
    return os_key

def mull_devcontainers(name):
    """Defines targets to generate devcontainer files

    Args:
      name: common target which generates all the files
    """
    write_targets = []
    for os_key in OS_VERSION_MAPPING:
        if os_key == "macos":
            continue
        os_name = os_key.split(":")[0]
        for llvm_version in OS_VERSION_MAPPING[os_key]:
            container_name = os_key.replace(":", "_") + "-llvm-" + llvm_version

            run_binary(
                name = "devcontainer_json_%s" % container_name,
                srcs = [":templates/devcontainers/%s/devcontainer.json.mustache" % os_name],
                outs = ["devcontainer_json_%s.json" % container_name],
                args = [
                    "--template",
                    "infrastructure/templates/devcontainers/%s/devcontainer.json.mustache" % os_name,
                    "--output",
                    "$(location devcontainer_json_%s.json)" % container_name,
                    "--os-key",
                    os_key,
                    "--llvm-version",
                    llvm_version,
                ],
                tool = ":devcontainer_json_gen",
            )
            run_binary(
                name = "dockerfile_%s" % container_name,
                srcs = [":templates/devcontainers/%s/Dockerfile.mustache" % os_name],
                outs = ["dockerfile_%s.dockerfile" % container_name],
                args = [
                    "--template",
                    "infrastructure/templates/devcontainers/%s/Dockerfile.mustache" % os_name,
                    "--output",
                    "$(location dockerfile_%s.dockerfile)" % container_name,
                    "--os-key",
                    _remap_os_key(os_key),
                    "--llvm-version",
                    llvm_version,
                ],
                tool = ":devcontainer_docker_gen",
            )

            write_source_files(
                name = "generate_devcontainer_%s_files" % container_name,
                files = {
                    "//.devcontainer:%s/devcontainer.json" % container_name: "devcontainer_json_%s.json" % container_name,
                    "//.devcontainer:%s/Dockerfile" % container_name: "dockerfile_%s.dockerfile" % container_name,
                },
                check_that_out_file_exists = False,
                suggested_update_target = name,
            )
            write_targets.append("generate_devcontainer_%s_files" % container_name)
    multirun(
        name = name,
        commands = write_targets,
        jobs = 0,  # Set to 0 to run in parallel, defaults to sequential
    )
