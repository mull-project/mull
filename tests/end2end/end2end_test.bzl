load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_files")
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS", "CC_PATHS", "CXX_PATHS")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")
load("@rules_multirun//:defs.bzl", "multirun")

def _mull_fmtlib_sqlite_report_impl(ctx):
    test_files = []

    for output in ctx.attr.target[OutputGroupInfo]:
        if output.endswith("-test"):
            test_file = ctx.attr.target[OutputGroupInfo][output].to_list()[0]
            test_files.append(test_file)

    out = ctx.actions.declare_file(ctx.attr.name + ".sqlite")

    cmds = []
    for test_file in test_files:
        cmds.append("%s --allow-surviving --reporters SQLite --report-name fmtlib %s" % (ctx.executable.mull_runner.path, test_file.path))

    cmds.append("cp fmtlib.sqlite %s" % out.path)

    script = ctx.actions.declare_file(ctx.attr.name + ".sh")
    ctx.actions.write(script, "\n".join(cmds), is_executable = True)

    ctx.actions.run_shell(
        inputs = [ctx.executable.mull_runner, script] + test_files,
        outputs = [out],
        command = script.path,
    )

    return [DefaultInfo(files = depset([out]))]

mull_fmtlib_sqlite_report = rule(
    implementation = _mull_fmtlib_sqlite_report_impl,
    attrs = {
        "target": attr.label(),
        "mull_runner": attr.label(executable = True, mandatory = True, cfg = "exec"),
    },
)

def _generate_ide_report_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name + ".txt")

    cmds = []
    cmds.append("%s -ide-reporter-show-killed -report-name fmt_ide_report %s" % (ctx.executable.mull_reporter.path, ctx.file.sqlite_report.path))

    # Make all the absolute paths relative for diff testing
    cmds.append("awk -F\"e2e_test_fmt/\" ' { print $NF } ' fmt_ide_report.txt > fmt_ide_report_relative_paths.txt")
    cmds.append("cp fmt_ide_report_relative_paths.txt %s" % out.path)

    script = ctx.actions.declare_file(ctx.attr.name + ".sh")
    ctx.actions.write(script, "\n".join(cmds), is_executable = True)

    ctx.actions.run_shell(
        inputs = [
            ctx.executable.mull_reporter,
            ctx.file.sqlite_report,
            script,
        ],
        outputs = [out],
        command = script.path,
    )

    return [DefaultInfo(files = depset([out]))]

generate_ide_report = rule(
    implementation = _generate_ide_report_impl,
    attrs = {
        "sqlite_report": attr.label(mandatory = True, allow_single_file = True),
        "mull_reporter": attr.label(executable = True, mandatory = True, cfg = "exec"),
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
    linux_commands = []
    macos_commands = []
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        cmake(
            name = "fmt_e2e_%s" % llvm_version,
            testonly = True,
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
            tags = ["llvm_%s" % llvm_version, "end2end"],
            targets = FMT_TEST_TARGETS,
        )

        mull_fmtlib_sqlite_report(
            name = "fmt_sqlite_report_%s" % llvm_version,
            testonly = True,
            mull_runner = "//:mull-runner-%s" % llvm_version,
            target = ":fmt_e2e_%s" % llvm_version,
        )

        generate_ide_report(
            name = "fmt_ide_report_%s" % llvm_version,
            testonly = True,
            mull_reporter = "//:mull-reporter-%s" % llvm_version,
            sqlite_report = "fmt_sqlite_report_%s" % llvm_version,
        )
        write_source_files(
            name = "update_end2end_fmtlib_test_files_linux_%s" % llvm_version,
            files = {
                ":fmtlib_expected_ide_report_linux.txt": "fmt_ide_report_%s" % llvm_version,
            },
            check_that_out_file_exists = False,
            testonly = True,
            target_compatible_with = ["@platforms//os:linux"],
            suggested_update_target = "%s_linux" % name,
            tags = ["llvm_%s" % llvm_version, "end2end"],
        )
        write_source_files(
            name = "update_end2end_fmtlib_test_files_macos_%s" % llvm_version,
            files = {
                ":fmtlib_expected_ide_report_macos.txt": "fmt_ide_report_%s" % llvm_version,
            },
            check_that_out_file_exists = False,
            testonly = True,
            target_compatible_with = ["@platforms//os:macos"],
            suggested_update_target = "%s_macos" % name,
            tags = ["llvm_%s" % llvm_version, "end2end"],
        )
        macos_commands.append("update_end2end_fmtlib_test_files_macos_%s" % llvm_version)
        linux_commands.append("update_end2end_fmtlib_test_files_linux_%s" % llvm_version)
    multirun(
        name = "%s_macos" % name,
        commands = macos_commands,
        jobs = 0,
        target_compatible_with = ["@platforms//os:macos"],
        testonly = True,
    )
    multirun(
        name = "%s_linux" % name,
        commands = linux_commands,
        jobs = 0,
        target_compatible_with = ["@platforms//os:linux"],
        testonly = True,
    )
