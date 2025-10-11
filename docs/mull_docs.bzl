"Generate devcontainer files"

load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_files")
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@rules_multirun//:defs.bzl", "multirun")

def _generate_rst(ctx):
    out = ctx.actions.declare_file(ctx.attr.name + ".rst")
    ctx.actions.run_shell(
        inputs = [ctx.executable.binary],
        outputs = [out],
        command = "%s %s %s" % (ctx.executable.binary.path, ctx.attr.arg, out.path),
    )

    return [DefaultInfo(files = depset([out]))]

mull_rst = rule(
    implementation = _generate_rst,
    attrs = {
        "binary": attr.label(mandatory = True, executable = True, cfg = "exec"),
        "arg": attr.string(mandatory = True),
    },
)

def _generate_cli_docs(binary, llvm_version, arg, rst):
    out_filename = "%s-%s-doc" % (binary, arg)
    mull_rst(
        name = out_filename,
        binary = "//:%s-%s" % (binary, llvm_version),
        arg = "--%s" % arg,
    )

    target_name = "write_%s_docs" % out_filename
    write_source_files(
        name = target_name,
        files = {
            rst: out_filename,
        },
    )
    return target_name

def mull_docs(name):
    """Defines targets to generate rst docs

    Args:
      name: common target which generates all the files
    """
    llvm_version = AVAILABLE_LLVM_VERSIONS[0]
    write_targets = []
    write_targets.append(_generate_cli_docs("mull-runner", llvm_version, "dump-cli", "command-line/generated/mull-runner-cli-options.rst"))
    write_targets.append(_generate_cli_docs("mull-reporter", llvm_version, "dump-cli", "command-line/generated/mull-reporter-cli-options.rst"))
    write_targets.append(_generate_cli_docs("mull-runner", llvm_version, "dump-mutators", "generated/Mutators.rst"))
    multirun(
        name = name,
        commands = write_targets,
        jobs = 0,
    )
