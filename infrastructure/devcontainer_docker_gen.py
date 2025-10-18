import argparse
import pystache

UBUNTU_CMD = """
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \\
  llvm-{LLVM_VERSION}-dev \\
  libclang-{LLVM_VERSION}-dev \\
  libc++-{LLVM_VERSION}-dev \\
  libc++abi-{LLVM_VERSION}-dev \\
  clang-{LLVM_VERSION} \\
  lld-{LLVM_VERSION}
"""

REDHAT_CMD = """
RUN yum update && yum install -y llvm-devel clang-devel clang lld
"""


def generate_llvm_install_cmds(os_name, llvm_versions):
    cmds = []
    cmd = UBUNTU_CMD
    if os_name == "redhat":
        cmd = REDHAT_CMD
    for llvm_version in llvm_versions:
        cmds.append(cmd.format(LLVM_VERSION=llvm_version))
    return "".join(cmds)


def main():
    parser = argparse.ArgumentParser(
        prog="generator", description="Generates devcontainer.json"
    )
    parser.add_argument(
        "--os-key",
        help="OS name + version",
        type=str,
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
        "--llvm-version",
        help="Target LLVM version",
        type=str,
    )
    args = parser.parse_args()

    os_name = args.os_key.split("/")[0].split(":")[0]

    template_args = {
        "OS_KEY": args.os_key,
        "INSTALL_LLVM_COMMANDS": generate_llvm_install_cmds(
            os_name, [args.llvm_version]
        ),
    }

    renderer = pystache.Renderer(missing_tags="strict")
    with open(args.template, "r") as t:
        result = renderer.render(t.read(), template_args)
        with open(args.output, "w") as f:
            f.write(result)


if __name__ == "__main__":
    main()
