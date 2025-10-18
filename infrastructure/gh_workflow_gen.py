#!/usr/bin/env python3

import argparse
import pystache


def main():
    parser = argparse.ArgumentParser(
        prog="generator", description="Generates devcontainer.json"
    )
    parser.add_argument(
        "--template",
        help="Template file path",
        type=str,
    )
    parser.add_argument(
        "--output",
        help="Output file path",
        type=str,
    )
    parser.add_argument(
        "--os-name",
        help="OS_NAME",
        type=str,
    )
    parser.add_argument(
        "--os-version",
        help="OS_NAME",
        type=str,
    )
    parser.add_argument(
        "--runner",
        help="GitHub runner",
        type=str,
    )
    parser.add_argument(
        "--package-extension",
        help="Package file extension",
        type=str,
    )
    parser.add_argument(
        "--arch",
        help="Target architecture",
        type=str,
    )
    parser.add_argument(
        "--llvm-versions",
        help="Supported LLVM versions",
        nargs="+",
    )
    args = parser.parse_args()

    template_args = {
        "OS_VERSION": args.os_version,
        "OS_RUNNER": args.runner,
        "OS_NAME": args.os_name,
        "OS_NAME_LOWER": args.os_name.lower().replace("-arm64", ""),
        "LLVM_VERSIONS": f"[{', '.join(args.llvm_versions)}]",
        "PACKAGE_EXTENSION": args.package_extension,
        "OS_ARCH": args.arch,
    }

    renderer = pystache.Renderer(missing_tags="strict")
    with open(args.template, "r") as t:
        result = renderer.render(t.read(), template_args)
        with open(args.output, "w") as f:
            f.write(result)


if __name__ == "__main__":
    main()
