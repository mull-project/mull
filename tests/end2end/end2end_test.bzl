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
