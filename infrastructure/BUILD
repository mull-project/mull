load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_binary")
load(":homebrew_config.bzl", "homebrew_config")
load(":mull_devcontainers.bzl", "mull_devcontainers")
load(":mull_github_workflows.bzl", "mull_github_workflows")

exports_files(["version.env"])

py_binary(
    name = "devcontainer_json_gen",
    srcs = ["devcontainer_json_gen.py"],
    deps = [
        requirement("pystache"),
    ],
)

py_binary(
    name = "devcontainer_docker_gen",
    srcs = ["devcontainer_docker_gen.py"],
    deps = [
        requirement("pystache"),
    ],
)

py_binary(
    name = "gh_workflow_gen",
    srcs = ["gh_workflow_gen.py"],
    deps = [
        requirement("pystache"),
    ],
)

mull_devcontainers(
    name = "generate_devcontainer_files",
)

mull_github_workflows(
    name = "generate_github_workflows",
)

homebrew_config(
    name = "homebrew_config",
)

py_binary(
    name = "generate-homebrew-formulas",
    srcs = ["generate_homebrew_formulas.py"],
    data = [":homebrew_config"],
    main = "generate_homebrew_formulas.py",
)
