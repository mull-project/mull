load("@available_llvm_versions//:mull_llvm_versions.bzl", "CC_PATHS", "CXX_PATHS")
load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")
load(":end2end_test.bzl", "generate_ide_report", "mull_fmtlib_sqlite_report")

# Picking some tests, not all the tests within fmtlib
FMT_TEST_TARGETS = [
    "chrono-test",
    "format-test",
    "ostream-test",
    "printf-test",
    "scan-test",
]

llvm_version = "18"

cmake(
    name = "fmt_e2e_%s" % llvm_version,
    build_args = ["-v"],
    build_data = [
        ":mull.yml",
    ],
    copts = [
        "-grecord-command-line",
        "-g",
    ],
    data = [
        "//:mull-cxx-ir-frontend-%s" % llvm_version,
    ],
    env = {
        "CC": CC_PATHS[llvm_version],
        "CXX": CXX_PATHS[llvm_version],
        "CFLAGS": "-fpass-plugin=$(execpath //:mull-cxx-ir-frontend-%s)" % llvm_version,
        "CXXFLAGS": "-fpass-plugin=$(execpath //:mull-cxx-ir-frontend-%s)" % llvm_version,
        "MULL_CONFIG": "$(execpath :mull.yml)",
    },
    lib_source = "@e2e_test_fmt//:all_srcs",
    out_binaries = FMT_TEST_TARGETS,
    targets = FMT_TEST_TARGETS,
)

mull_fmtlib_sqlite_report(
    name = "fmt_sqlite_report_%s" % llvm_version,
    mull_runner = "//:mull-runner-%s" % llvm_version,
    target = ":fmt_e2e_%s" % llvm_version,
)

generate_ide_report(
    name = "fmt_ide_report_%s" % llvm_version,
    mull_reporter = "//:mull-reporter-%s" % llvm_version,
    sqlite_report = "fmt_sqlite_report_%s" % llvm_version,
)

diff_test(
    name = "fmtlib_ide_report_test_%s" % llvm_version,
    file1 = ":fmtlib_expected_ide_report.txt",
    file2 = "fmt_ide_report_%s" % llvm_version,
)
