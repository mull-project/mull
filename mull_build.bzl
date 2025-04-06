# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS", "EXACT_VERSION_MAPPING")
load("@bazel_skylib//rules:expand_template.bzl", "expand_template")
load("@mull_package_info//:mull_package_info.bzl", "GIT_SHA", "MULL_DESCRIPTION", "MULL_DOCS_URL", "MULL_HOMEPAGE", "MULL_SUPPORT_URL", "MULL_VERSION")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

def mull_build(name):
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        expand_template(
            name = "mull_version_%s" % llvm_version,
            out = "Version.cpp",
            substitutions = {
                "@GIT_COMMIT@": GIT_SHA,
                "@PROJECT_DESCRIPTION@": MULL_DESCRIPTION,
                "@PROJECT_HOMEPAGE_URL@": MULL_HOMEPAGE,
                "@PROJECT_DOCS_URL@": MULL_DOCS_URL,
                "@PROJECT_SUPPORT_URL@": MULL_SUPPORT_URL,
                "@PROJECT_VERSION@": MULL_VERSION,
                "@LLVM_VERSION@": EXACT_VERSION_MAPPING[llvm_version],
            },
            template = ":lib/Version.cpp",
        )
        cc_library(
            name = "libmull_%s" % llvm_version,
            srcs = native.glob(["lib/**/*.cpp"], exclude = ["lib/Version.cpp"]) + [":mull_version_%s" % llvm_version],
            hdrs = native.glob(["include/**/*.h"]),
            includes = ["include"],
            deps = [
                "@mull_irm_%s//:irm" % llvm_version,
                "@json11",
                "@llvm_%s//:libclang" % llvm_version,
                "@llvm_%s//:libllvm" % llvm_version,
                "@reproc//:reproc++",
                "@spdlog",
                "@sqlite3",
            ],
        )

        cc_library(
            name = "libmull_cli_options_%s" % llvm_version,
            srcs = ["tools/CLIOptions/CLIOptions.cpp"],
            hdrs = ["tools/CLIOptions/CLIOptions.h"],
            deps = [
                ":libmull_%s" % llvm_version,
                "@llvm_%s//:libllvm" % llvm_version,
            ],
        )

        cc_library(
            name = "libmull_runner_%s" % llvm_version,
            srcs = native.glob(["tools/mull-runner/*.cpp"]),
            hdrs = native.glob(["tools/mull-runner/*.h"]),
            deps = [
                "libmull_%s" % llvm_version,
                ":libmull_cli_options_%s" % llvm_version,
            ],
        )

        cc_library(
            name = "libmull_reporter_%s" % llvm_version,
            srcs = native.glob(["tools/mull-reporter/*.cpp"]),
            hdrs = native.glob(["tools/mull-reporter/*.h"]),
            deps = [
                "libmull_%s" % llvm_version,
                ":libmull_cli_options_%s" % llvm_version,
            ],
        )

        cc_binary(
            name = "mull-cxx-ir-frontend-%s" % llvm_version,
            srcs = native.glob(["tools/mull-ir-frontend/*.cpp"]),
            linkshared = True,
            deps = [
                ":libmull_%s" % llvm_version,
                "@llvm_%s//:libclang" % llvm_version,
            ],
            tags = ["llvm_%s" % llvm_version],
        )

        native.genrule(
            name = "mull-ir-frontend-%s-gen" % llvm_version,
            srcs = [":mull-cxx-ir-frontend-%s" % llvm_version],
            outs = ["mull-ir-frontend-%s" % llvm_version],
            cmd = "cp $(SRCS) $(OUTS)",
        )

        cc_binary(
            name = "mull-cxx-ast-frontend-%s" % llvm_version,
            srcs = native.glob([
                "tools/mull-cxx-frontend/src/*.cpp",
                "tools/mull-cxx-frontend/src/*.h",
            ]),
            linkshared = True,
            deps = [
                ":libmull_%s" % llvm_version,
                "@llvm_%s//:libclang" % llvm_version,
            ],
            tags = ["llvm_%s" % llvm_version],
        )

        native.genrule(
            name = "mull-ast-frontend-%s-gen" % llvm_version,
            srcs = [":mull-cxx-ast-frontend-%s" % llvm_version],
            outs = ["mull-ast-frontend-%s" % llvm_version],
            cmd = "cp $(SRCS) $(OUTS)",
        )

        cc_binary(
            name = "mull-runner-%s" % llvm_version,
            deps = [
                ":libmull_runner_%s" % llvm_version,
            ],
            tags = ["llvm_%s" % llvm_version],
        )

        cc_binary(
            name = "mull-reporter-%s" % llvm_version,
            deps = [
                ":libmull_reporter_%s" % llvm_version,
            ],
            tags = ["llvm_%s" % llvm_version],
        )
