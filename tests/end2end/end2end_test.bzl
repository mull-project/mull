def _mull_end2end_test_impl(ctx):
    print(dir(ctx.attr.target[OutputGroupInfo]))

    args_test = ctx.attr.target[OutputGroupInfo]["args-test"].to_list()[0]
    print(args_test.path)

    # print(dir(ctx.attr.target[ForeignCcDepsInfo]))
    cmds = []
    script = ctx.actions.declare_file("publish_22.sh")
    ctx.actions.write(script, "".join(cmds), is_executable = True)

    return [DefaultInfo(files = depset([script]))]

mull_end2end = rule(
    implementation = _mull_end2end_test_impl,
    attrs = {
        "target": attr.label(),
    },
)
