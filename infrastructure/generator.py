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

    args = parser.parse_args()
    if args.cmd == "cmake":
        cmake(args)
    if args.cmd == "devcontainer":
        devcontainer(args)


if __name__ == "__main__":
    main()
