load("@available_llvm_versions//:mull_llvm_versions.bzl", "CXX_PATHS")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

TEST_TARGETS = [
    "args-test",
    "assert-test",
    "base-test",
    "chrono-test",
    "color-test",
    "compile-fp-test",
    "compile-test",
    "enforce-checks-test",
    "format-test",
    "gtest-extra-test",
    "no-builtin-types-test",
    "os-test",
    "ostream-test",
    "printf-test",
    "ranges-test",
    "scan-test",
    "std-test",
    "unicode-test",
    "xchar-test",
]

filegroup(
    name = "all_srcs",
    srcs = glob(["fmt/**"]),
)

llvm_version = "18"

cmake(
    name = "fmt_e2e_test_%s" % llvm_version,
    build_args = ["-v"],
    build_data = [
        ":mull.yml",
    ],
    copts = [
        "-gdwarf-5",
        "-gembed-source",
        "-grecord-command-line",
        "-g",
        "-fpass-plugin=/private/var/tmp/_bazel_alexdenisov/a287b22490debf48c4aafe63e4cbb281/execroot/_main/bazel-out/darwin_arm64-fastbuild/bin/libmull-cxx-ir-frontend-18.dylib",
    ],
    # data = [
    #     "//:mull-cxx-ir-frontend-%s" % llvm_version,
    # ],
    env = {
        "CXX": CXX_PATHS[llvm_version],
        "MULL_CONFIG": "$(execpath :mull.yml)",
    },
    generate_args = [
        "-G Ninja",
    ],
    # lib_source = "@e2e_test_fmt//:all_srcs",
    lib_source = ":all_srcs",
    out_binaries = TEST_TARGETS,
    targets = TEST_TARGETS,
)

# mull_end2end(
#     name = "fmt_test",
#     target = ":e2e_test_fmt",
# )
