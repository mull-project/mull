#!/usr/bin/env python3

import argparse
import pystache
import pystache.renderer


def cmake(args):
    template_name = f"infrastructure/templates/cmake-presets/{args.os}.mustache"
    template_args = {"LLVM_VERSION": args.llvm_version}
    with open(template_name, "r") as t:
        result = pystache.render(t.read(), template_args)
        with open('CMakePresets.json', "w") as f:
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
    args = parser.parse_args()
    if args.cmd == "cmake":
        cmake(args)


if __name__ == "__main__":
    main()
