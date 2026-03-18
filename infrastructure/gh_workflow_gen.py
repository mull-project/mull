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

    os_name_lower = args.os_name.lower().replace("-arm64", "")
    if os_name_lower == "rhel":
        major_version = args.os_version.split(".")[0]
        base_image = f"redhat/ubi{major_version}:{args.os_version}"
        pre_install_cmd = "true"  # curl-minimal is pre-installed in UBI
        install_cmd = "dnf install -y"
        cloudsmith_setup_script = "setup.rpm.sh"
    else:
        base_image = f"{os_name_lower}:{args.os_version}"
        pre_install_cmd = "apt-get update && apt-get install -y curl ca-certificates"
        install_cmd = "apt-get install -y"
        cloudsmith_setup_script = "setup.deb.sh"

    template_args = {
        "OS_VERSION": args.os_version,
        "OS_RUNNER": args.runner,
        "OS_NAME": args.os_name,
        "OS_NAME_LOWER": os_name_lower,
        "LLVM_VERSIONS": f"[{', '.join(args.llvm_versions)}]",
        "PACKAGE_EXTENSION": args.package_extension,
        "OS_ARCH": args.arch,
        "BASE_IMAGE": base_image,
        "PRE_INSTALL_CMD": pre_install_cmd,
        "INSTALL_CMD": install_cmd,
        "CLOUDSMITH_SETUP_SCRIPT": cloudsmith_setup_script,
    }

    renderer = pystache.Renderer(missing_tags="strict")
    with open(args.template, "r") as t:
        result = renderer.render(t.read(), template_args)
        with open(args.output, "w") as f:
            f.write(result)


if __name__ == "__main__":
    main()
