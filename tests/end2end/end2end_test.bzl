load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS", "CC_PATHS", "CXX_PATHS")
load("@bazel_skylib//lib:shell.bzl", "shell")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

def _mull_e2e_test_impl(ctx):
    """Test rule that generates IDE report and compares to expected.

    By making generation part of the test action (not build action),
    retries will regenerate the output.
    """
    test_files = []
    for output in ctx.attr.fmt_target[OutputGroupInfo]:
        if output.endswith("-test"):
            test_file = ctx.attr.fmt_target[OutputGroupInfo][output].to_list()[0]
            test_files.append(test_file)

    test_file_paths = " ".join([shell.quote(f.short_path) for f in test_files])

    script_content = """#!/bin/bash
set -e

MULL_RUNNER="{mull_runner}"
MULL_REPORTER="{mull_reporter}"
MULL_CONFIG="{mull_config}"
EXPECTED="{expected}"
TEST_FILES=({test_files})

export MULL_CONFIG

# Step 1: Run mull-runner on each test binary to generate SQLite report
for test_file in "${{TEST_FILES[@]}}"; do
    "$MULL_RUNNER" --allow-surviving --reporters SQLite --report-name fmtlib -minimum-timeout 500 "$test_file"
done

# Step 2: Run mull-reporter to generate IDE report
"$MULL_REPORTER" --allow-surviving -ide-reporter-show-killed -report-name fmt_ide_report fmtlib.sqlite

# Step 3: Make paths relative for comparison
awk -F"e2e_test_fmt/" ' {{ print $NF }} ' fmt_ide_report.txt > fmt_ide_report_relative_paths.txt

# Step 4: Compare with expected
if diff -u "$EXPECTED" fmt_ide_report_relative_paths.txt; then
    echo "PASS: Output matches expected"
    exit 0
else
    echo "FAIL: Output differs from expected"
    echo "=== Generated output ==="
    cat fmt_ide_report_relative_paths.txt
    exit 1
fi
""".format(
        mull_runner = ctx.executable.mull_runner.short_path,
        mull_reporter = ctx.executable.mull_reporter.short_path,
        mull_config = ctx.file.mull_config.short_path,
        expected = ctx.file.expected.short_path,
        test_files = test_file_paths,
    )

    script = ctx.actions.declare_file(ctx.attr.name + "_test.sh")
    ctx.actions.write(script, script_content, is_executable = True)

    runfiles = ctx.runfiles(
        files = [
            ctx.executable.mull_runner,
            ctx.executable.mull_reporter,
            ctx.file.mull_config,
            ctx.file.expected,
        ] + test_files,
    )
    runfiles = runfiles.merge(ctx.attr.mull_runner[DefaultInfo].default_runfiles)
    runfiles = runfiles.merge(ctx.attr.mull_reporter[DefaultInfo].default_runfiles)

    return [DefaultInfo(
        executable = script,
        runfiles = runfiles,
    )]

mull_e2e_test = rule(
    implementation = _mull_e2e_test_impl,
    test = True,
    attrs = {
        "fmt_target": attr.label(mandatory = True),
        "mull_runner": attr.label(executable = True, mandatory = True, cfg = "exec"),
        "mull_reporter": attr.label(executable = True, mandatory = True, cfg = "exec"),
        "mull_config": attr.label(mandatory = True, allow_single_file = True),
        "expected": attr.label(mandatory = True, allow_single_file = True),
    },
)

FMT_TEST_TARGETS = [
    "chrono-test",
    "format-test",
    "ostream-test",
    "printf-test",
    "scan-test",
]

def define_end2end_test_targets(name):
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        cmake(
            name = "fmt_e2e_%s" % llvm_version,
            testonly = True,
            build_args = ["-v"],
            build_data = [
                ":mull.yml",
                # listing the dependency explicitly to ensure this get cached correctly
                "@llvm_%s//:clang" % llvm_version,
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
            tags = ["llvm_%s" % llvm_version, "end2end"],
            targets = FMT_TEST_TARGETS,
            generate_args = ["-GNinja"],
        )

        # Linux x86_64
        mull_e2e_test(
            name = "fmtlib_e2e_test_linux_x86_64_%s" % llvm_version,
            fmt_target = ":fmt_e2e_%s" % llvm_version,
            mull_runner = "//rust/mull-tools:mull-runner-%s" % llvm_version,
            mull_reporter = "//rust/mull-tools:mull-reporter-%s" % llvm_version,
            mull_config = ":mull.yml",
            expected = ":fmtlib_expected_ide_report_linux_x86_64.txt",
            target_compatible_with = ["@platforms//os:linux", "@platforms//cpu:x86_64"],
            tags = ["llvm_%s" % llvm_version, "end2end"],
        )

        # Linux aarch64
        mull_e2e_test(
            name = "fmtlib_e2e_test_linux_aarch64_%s" % llvm_version,
            fmt_target = ":fmt_e2e_%s" % llvm_version,
            mull_runner = "//rust/mull-tools:mull-runner-%s" % llvm_version,
            mull_reporter = "//rust/mull-tools:mull-reporter-%s" % llvm_version,
            mull_config = ":mull.yml",
            expected = ":fmtlib_expected_ide_report_linux_aarch64.txt",
            target_compatible_with = ["@platforms//os:linux", "@platforms//cpu:aarch64"],
            tags = ["llvm_%s" % llvm_version, "end2end"],
        )

        # macOS
        mull_e2e_test(
            name = "fmtlib_e2e_test_macos_%s" % llvm_version,
            fmt_target = ":fmt_e2e_%s" % llvm_version,
            mull_runner = "//rust/mull-tools:mull-runner-%s" % llvm_version,
            mull_reporter = "//rust/mull-tools:mull-reporter-%s" % llvm_version,
            mull_config = ":mull.yml",
            expected = ":fmtlib_expected_ide_report_macos.txt",
            target_compatible_with = ["@platforms//os:macos"],
            tags = ["llvm_%s" % llvm_version, "end2end"],
        )
