#!/usr/bin/env python3

import argparse
import pystache
import os
import shutil

SUPPORTED_PLATFORMS = {
    "ubuntu": {
        "versions": {
            "20.04": [12],
            "22.04": [13, 14, 15],
            "24.04": [14, 15, 16, 17, 18],
        },
        "template": "ubuntu",
        "runner": "ubuntu-latest",
    },
    "ubuntu-arm64": {
        "versions": {
            "20.04": [12],
            "22.04": [13, 14, 15],
            "24.04": [14, 15, 16, 17, 18],
        },
        "template": "ubuntu",
        "runner": "ubuntu-24.04-arm",
    },
    "macos": {
        "versions": {"latest": [14, 16, 17, 18]},
        "template": "macos",
        "runner": "macos-latest",
    },
}


def cmake(args):
    os_specific_args = {
        "ubuntu": {
            "CC": f"clang-{args.llvm_version}",
            "CXX": f"clang++-{args.llvm_version}",
            "CMAKE_PREFIX_PATH": f"/usr/lib/llvm-{args.llvm_version}/cmake/;/usr/lib/cmake/clang-{args.llvm_version}/",
        },
        "macos": {
            "CC": f"/opt/homebrew/opt/llvm@{args.llvm_version}/bin/clang",
            "CXX": f"/opt/homebrew/opt/llvm@{args.llvm_version}/bin/clang++",
            "CMAKE_PREFIX_PATH": f"/opt/homebrew/opt/llvm@{args.llvm_version}/lib/cmake/llvm/;/opt/homebrew/opt/llvm@{args.llvm_version}/lib/cmake/clang/",
        },
    }
    template_name = f"infrastructure/templates/cmake-presets/CMakePresets.json.mustache"
    template_args = os_specific_args[args.os]
    template_args["LLVM_VERSION"] = args.llvm_version
    template_args["OS_NAME"] = args.os
    renderer = pystache.Renderer(missing_tags="strict")
    with open(template_name, "r") as t:
        result = renderer.render(t.read(), template_args)
        with open("CMakePresets.json", "w") as f:
            f.write(result)


def vscode(args):
    os_specific_args = {
        "ubuntu": {
            "INSTALL_PATH": f"/usr/lib/llvm-{args.llvm_version}",
            "COMPILER": f"/usr/bin/clang-{args.llvm_version}"
        },
        "macos": {
            "INSTALL_PATH": f"opt/homebrew/opt/llvm@{args.llvm_version}",
            "COMPILER": f"/opt/homebrew/opt/llvm@{args.llvm_version}/bin/clang",
        },
    }

    template_args = os_specific_args[args.os]
    template_name = f"infrastructure/templates/vscode/c_cpp_properties.json.mustache"
    renderer = pystache.Renderer(missing_tags="strict")
    with open(template_name, "r") as t:
        result = renderer.render(t.read(), template_args)
        with open(".vscode/c_cpp_properties.json", "w") as f:
            f.write(result)


def devcontainers(args):
    shutil.rmtree(".devcontainer")
    for os_name in SUPPORTED_PLATFORMS:
        template_folder = f"infrastructure/templates/devcontainers/{os_name}"
        if not os.path.exists(template_folder):
            continue
        platform = SUPPORTED_PLATFORMS[os_name]
        for os_version, llvm_versions in platform["versions"].items():
            for llvm_version in llvm_versions:
                container_folder = (
                    f".devcontainer/{os_name}_{os_version}_{llvm_version}"
                )
                os.makedirs(container_folder, exist_ok=True)

                template_args = {
                    "LLVM_VERSION": llvm_version,
                    "OS_NAME": os_name,
                    "OS_VERSION": os_version,
                }

                renderer = pystache.Renderer(missing_tags="strict")
                for template in ["devcontainer.json", "Dockerfile"]:
                    template_name = f"{template_folder}/{template}.mustache"
                    result_filename = f"{container_folder}/{template}"
                    with open(template_name, "r") as t:
                        result = renderer.render(t.read(), template_args)
                        with open(result_filename, "w") as f:
                            f.write(result)


def gh_workflows(args):
    template_folder = f"infrastructure/templates/github-actions/"
    workflow_folder = f".github/workflows/"

    for os_name in SUPPORTED_PLATFORMS.keys():
        strategies = []
        os_versions = SUPPORTED_PLATFORMS[os_name]["versions"]
        template_name = SUPPORTED_PLATFORMS[os_name]["template"]
        for os_version in sorted(os_versions):
            for llvm_version in os_versions[os_version]:
                arg = {
                    "OS_NAME": template_name,
                    "OS_VERSION": os_version,
                    "LLVM_VERSION": llvm_version,
                }
                strategies.append(arg)

        template_args = {
            "strategy": strategies,
            "OS_NAME": os_name.capitalize(),
            "OS_NAME_LOWER": os_name,
            "OS_RUNNER": SUPPORTED_PLATFORMS[os_name]["runner"],
        }
        renderer = pystache.Renderer(missing_tags="strict")
        template_file = f"{template_folder}/ci-{template_name}.yml.mustache"
        result_filename = f"{workflow_folder}/ci-{os_name}.yml"
        with open(template_file, "r") as t:
            result = renderer.render(t.read(), template_args)
            with open(result_filename, "w") as f:
                f.write(result)


def main():
    parser = argparse.ArgumentParser(
        prog="generator", description="Generates various infra files for Mull"
    )
    subparsers = parser.add_subparsers(
        title="subcommands", help="available subcommands", dest="cmd"
    )

    parser_cmake = subparsers.add_parser(
        "cmake", help="Generates CMake preset file")
    parser_cmake.add_argument(
        "--os",
        choices=("ubuntu", "macos"),
        help="Select OS for which to generate preset",
        type=str,
    )
    parser_cmake.add_argument("--llvm_version", type=int)

    parser_vscode = subparsers.add_parser(
        "vscode", help="Generates VSCode settings")
    parser_vscode.add_argument(
        "--os",
        choices=("ubuntu", "macos"),
        help="Select OS for which to generate preset",
        type=str,
    )
    parser_vscode.add_argument("--llvm_version", type=int)

    subparsers.add_parser("devcontainers", help="Generates devcontainer files")
    subparsers.add_parser("github_workflows",
                          help="Generates GitHub workflow files")
    subparsers.add_parser(
        "all", help="Combines devcontainers and github_workflows")

    args = parser.parse_args()
    if args.cmd == "cmake":
        cmake(args)
    if args.cmd == "vscode":
        vscode(args)
    if args.cmd == "devcontainers":
        devcontainers(args)
    if args.cmd == "github_workflows":
        gh_workflows(args)
    if args.cmd == "all":
        gh_workflows(args)
        devcontainers(args)


if __name__ == "__main__":
    main()
