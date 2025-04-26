#!/usr/bin/env python3

import argparse
import pystache
import os

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
    args = parser.parse_args()

    template_args = {
        "OS_KEY" : args.os_key
    }

    renderer = pystache.Renderer(missing_tags="strict")
    with open(args.template, "r") as t:
        result = renderer.render(t.read(), template_args)
        with open(args.output, "w") as f:
            f.write(result)


if __name__ == "__main__":
    main()
