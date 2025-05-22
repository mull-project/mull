load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_binary")
load(":mull_devcontainers.bzl", "mull_devcontainers")

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

mull_devcontainers(
    name = "generate_devcontainer_files",
)
