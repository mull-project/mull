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

    anchor = "include/fmt/base.h"
    anchor_files = [f for f in ctx.files.fmt_srcs if f.short_path.endswith("/" + anchor)]
    if len(anchor_files) != 1:
        fail("expected exactly one %s in fmt_srcs" % anchor)
    fmt_src_root = anchor_files[0].short_path[:-len(anchor)]

    script_content = """#!/bin/bash
set -e

MULL_RUNNER="{mull_runner}"
MULL_REPORTER="{mull_reporter}"
MULL_CONFIG="{mull_config}"
SQLITE3="{sqlite3}"
EXPECTED="{expected}"
TEST_FILES=({test_files})
TEST_ARGS=({test_args})

export MULL_CONFIG

# Step 1: Run mull-runner on each test binary to generate SQLite report
for test_file in "${{TEST_FILES[@]}}"; do
    echo "Testing $test_file"
    "$MULL_RUNNER" --allow-surviving --reporters SQLite --report-name fmtlib -minimum-timeout 500 "$test_file" "${{TEST_ARGS[@]}}"
done

# Step 2: Point mutants at the fmt sources shipped with the test
ANCHOR_PATH="$(realpath "{fmt_src_root}{anchor}")"
FMT_SRC_ROOT="${{ANCHOR_PATH%{anchor}}}"
BUILD_SRC_ROOT=$("$SQLITE3" fmtlib.sqlite "SELECT substr(filename, 1, length(filename) - length('{anchor}')) FROM mutant WHERE filename LIKE '%/{anchor}' LIMIT 1;")
if [ -z "$BUILD_SRC_ROOT" ]; then
    echo "FAIL: cannot find {anchor} in fmtlib.sqlite"
    exit 1
fi
# mull-reporter takes mutant locations from mutant_id
"$SQLITE3" fmtlib.sqlite "UPDATE mutant SET mutant_id = replace(mutant_id, '$BUILD_SRC_ROOT', '$FMT_SRC_ROOT'), filename = replace(filename, '$BUILD_SRC_ROOT', '$FMT_SRC_ROOT'), directory = replace(directory, '$BUILD_SRC_ROOT', '$FMT_SRC_ROOT');"

# Step 3: Run mull-reporter to generate IDE report
"$MULL_REPORTER" --allow-surviving -ide-reporter-show-killed -report-name fmt_ide_report fmtlib.sqlite

# Keep raw results for investigating flaky runs
if [ -n "${{TEST_UNDECLARED_OUTPUTS_DIR:-}}" ]; then
    cp fmtlib.sqlite fmt_ide_report.txt "$TEST_UNDECLARED_OUTPUTS_DIR/"
fi

# Step 4: Make paths relative for comparison
while IFS= read -r line; do
    printf '%s\\n' "${{line//"$FMT_SRC_ROOT"/}}"
done < fmt_ide_report.txt > fmt_ide_report_relative_paths.txt

# Step 5: Compare with expected
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
        sqlite3 = ctx.executable.sqlite3.short_path,
        fmt_src_root = fmt_src_root,
        anchor = anchor,
        expected = ctx.file.expected.short_path,
        test_files = test_file_paths,
        test_args = " ".join([shell.quote(a) for a in ctx.attr.test_args]),
    )

    script = ctx.actions.declare_file(ctx.attr.name + "_test.sh")
    ctx.actions.write(script, script_content, is_executable = True)

    runfiles = ctx.runfiles(
        files = [
            ctx.executable.mull_runner,
            ctx.executable.mull_reporter,
            ctx.executable.sqlite3,
            ctx.file.mull_config,
            ctx.file.expected,
        ] + test_files + ctx.files.fmt_srcs,
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
        "sqlite3": attr.label(executable = True, mandatory = True, cfg = "exec"),
        "fmt_srcs": attr.label(mandatory = True, allow_files = True),
        "mull_config": attr.label(mandatory = True, allow_single_file = True),
        "expected": attr.label(mandatory = True, allow_single_file = True),
        "test_args": attr.string_list(),
    },
)

FMT_TEST_TARGETS = [
    "chrono-test",
    "format-test",
    "ostream-test",
    "printf-test",
    "scan-test",
]

# Platform configurations: (os, arch, target_compatible_with)
E2E_PLATFORMS = [
    ("linux", "x86_64", ["@platforms//os:linux", "@platforms//cpu:x86_64"]),
    ("linux", "aarch64", ["@platforms//os:linux", "@platforms//cpu:aarch64"]),
    ("macos", "aarch64", ["@platforms//os:macos"]),
]

def define_end2end_test_targets(name):
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        # TODO: fmtlib fails to build with Clang 21 on macOS
        skip_on_macos = llvm_version == "21"

        cmake(
            name = "fmt_e2e_%s" % llvm_version,
            target_compatible_with = select({
                "@platforms//os:macos": ["@platforms//:incompatible"],
                "//conditions:default": [],
            }) if skip_on_macos else [],
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
            # toolchains_llvm adds -fno-cxx-modules, which older system clangs reject
            features = [
                "-local-%s_disable_cpp_modules" % platform
                for platform in ["aarch64-darwin", "aarch64-linux", "x86_64-linux"]
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

        expected_suffix = "_macro" if int(llvm_version) >= 22 else ""
        for os, arch, target_compatible_with in E2E_PLATFORMS:
            if os == "macos" and skip_on_macos:
                continue

            mull_e2e_test(
                name = "fmtlib_e2e_test_%s_%s_%s" % (os, arch, llvm_version),
                fmt_target = ":fmt_e2e_%s" % llvm_version,
                mull_runner = "//rust/mull-tools:mull-runner-%s" % llvm_version,
                mull_reporter = "//rust/mull-tools:mull-reporter-%s" % llvm_version,
                sqlite3 = "//tests/integration:sqlite3",
                fmt_srcs = "@e2e_test_fmt//:all_srcs",
                mull_config = ":mull.yml",
                expected = ":fmtlib_expected_ide_report_%s_%s%s.txt" % (os, arch, expected_suffix),
                # macOS 26 system frameworks leave FE_INEXACT set before main
                test_args = ["--gtest_filter=-float_test.isnan"] if os == "macos" else [],
                flaky = True,
                target_compatible_with = target_compatible_with,
                tags = ["llvm_%s" % llvm_version, "end2end"],
            )
