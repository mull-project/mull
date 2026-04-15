# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

def mull_build(name):
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        cc_library(
            name = "libmull_%s" % llvm_version,
            srcs = native.glob(["lib/**/*.cpp"]),
            hdrs = native.glob(["include/**/*.h"]),
            includes = ["include"],
            deps = [
                "@mull_irm_%s//:irm" % llvm_version,
                "@llvm_%s//:libclang" % llvm_version,
                "@llvm_%s//:libllvm" % llvm_version,
                "@sqlite3",
                "//rust/mull-cxx-bridge",
                "//rust/mull-cxx-bridge:bridge",
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
            name = "mull-instrument-%s" % llvm_version,
            srcs = ["tools/mull-instrument/mull-instrument.cpp"],
            deps = [
                ":libmull_%s" % llvm_version,
                "@llvm_%s//:libllvm" % llvm_version,
            ],
            tags = ["llvm_%s" % llvm_version],
        )

    # Documentation tool - uses single LLVM version only
    latest_llvm = AVAILABLE_LLVM_VERSIONS[0]
    cc_binary(
        name = "mull-dump-mutators",
        srcs = ["tools/mull-dump-mutators/mull-dump-mutators.cpp"],
        deps = [
            ":libmull_%s" % latest_llvm,
        ],
    )
