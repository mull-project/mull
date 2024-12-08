#!/usr/bin/env python3

import argparse
import pystache
import os


def cmake(args):
    template_name = f"infrastructure/templates/cmake-presets/{args.os}.mustache"
    template_args = {"LLVM_VERSION": args.llvm_version}
    with open(template_name, "r") as t:
        result = pystache.render(t.read(), template_args)
        with open('CMakePresets.json', "w") as f:
            f.write(result)


def devcontainers(os_name, os_version, llvm_version):
    template_folder = f"infrastructure/templates/devcontainers/{os_name}"
    container_folder = f".devcontainer/{os_name}_{os_version}_{llvm_version}"
    os.makedirs(container_folder, exist_ok=True)

    template_args = {"LLVM_VERSION": llvm_version,
                     "OS": os_name, "OS_VERSION": os_version}

    for template in ['devcontainer.json', "Dockerfile"]:
        template_name = f"{template_folder}/{template}.mustache"
        result_filename = f"{container_folder}/{template}"
        with open(template_name, "r") as t:
            result = pystache.render(t.read(), template_args)
            with open(result_filename, "w") as f:
                f.write(result)


def devcontainer(args):
    devcontainers(args.os, args.os_version, args.llvm_version)

    # UBUNTU_VERSION: ["22.04", "24.04"]
    # LLVM_VERSION: [14, 15]
    # include:
    #   - UBUNTU_VERSION: "20.04"
    #     LLVM_VERSION: 12
    #   - UBUNTU_VERSION: "22.04"
    #     LLVM_VERISON: 13
    #   - UBUNTU_VERSION: "24.04"
    #     LLVM_VERSION: 16
    #   - UBUNTU_VERSION: "24.04"
    #     LLVM_VERSION: 17


def gh_workflows(args):
    supported_platforms = {
        "ubuntu": {
            "20.04": [12],
            "22.04": [13]
        }
    }
    template_folder = f"infrastructure/templates/github-actions/"
    workflow_folder = f".github/workflows/"

    strategies = []
    for os_version in sorted(supported_platforms["ubuntu"].keys()):
        for llvm_version in supported_platforms["ubuntu"][os_version]:
            arg = {"OS_VERSION": os_version, "LLVM_VERSION": llvm_version}
            strategies.append(arg)

    template_args = {"strategy": strategies}
    renderer = pystache.Renderer(missing_tags="strict")
    for template in ['ci-linux.yml']:
        template_name = f"{template_folder}/{template}.mustache"
        result_filename = f"{workflow_folder}/{template}"
        with open(template_name, "r") as t:
            result = renderer.render(t.read(), template_args)
            with open(result_filename, "w") as f:
                f.write(result)


def main():
    parser = argparse.ArgumentParser(
        prog='generator', description='Generates various infra files for Mull')
    subparsers = parser.add_subparsers(
        title='subcommands', help='available subcommands', dest="cmd")

    parser_cmake = subparsers.add_parser(
        'cmake', help='Generates CMake preset file')
    parser_cmake.add_argument('--os', choices=('ubuntu',),
                              help='Select OS for which to generate preset', type=str)
    parser_cmake.add_argument("--llvm_version", type=int)

    parser_devcontainer = subparsers.add_parser(
        'devcontainer', help='Generates devcontainer files')
    parser_devcontainer.add_argument('--os', choices=('ubuntu',),
                                     help='Select OS for which to generate preset', type=str)
    parser_devcontainer.add_argument("--os_version", type=str)
    parser_devcontainer.add_argument("--llvm_version", type=int)

    gh_workflow_parser = subparsers.add_parser(
        'github_workflows', help='Generates GitHub workflow files')

    args = parser.parse_args()
    if args.cmd == "cmake":
        cmake(args)
    if args.cmd == "devcontainer":
        devcontainer(args)
    if args.cmd == "github_workflows":
        gh_workflows(args)


if __name__ == "__main__":
    main()
