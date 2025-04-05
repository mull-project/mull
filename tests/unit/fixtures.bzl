def _fixture_impl(ctx):
    # llvm_version = "18"
    out = ctx.actions.declare_file(ctx.label.name)

    # this is a hack for a single test case
    inc = "-I" + ctx.file.fixture.dirname + "/include"
    args = ctx.attr.args + ["-fdebug-prefix-map=$PWD=./", inc] + [ctx.file.fixture.path, "-o", out.path]
    ctx.actions.run_shell(
        inputs = [ctx.executable.compiler, ctx.file.fixture] + ctx.files.data,
        outputs = [out],
        progress_message = "Generating fixture %s" % out.short_path,
        command = "%s %s" % (ctx.executable.compiler.path, " ".join(args)),
    )

    return [DefaultInfo(files = depset([out]))]

fixture = rule(
    implementation = _fixture_impl,
    attrs = {
        "compiler": attr.label(executable = True, mandatory = True, cfg = "exec"),
        "args": attr.string_list(mandatory = True),
        "fixture": attr.label(mandatory = True, allow_single_file = True),
        "data": attr.label_list(allow_files = True),
    },
)

HEADER = """
// !!! Autogenerated file !!!
namespace mull {
namespace fixtures {
"""

FOOTER = """
} // namespace fixtures
} // namespace mull
"""

CODE = """
__attribute__((used)) static const char* {NAME}_path() {{
  return "{PATH}";
}}
"""

def _fixtures_header_impl(ctx):
    llvm_version = "18"
    chunks = []
    for fixture in ctx.files.fixtures:
        name = fixture.short_path.replace("/", "_").replace(".", "_")
        name = name.replace("_" + llvm_version, "")
        chunks.append(CODE.format(NAME = name, PATH = fixture.short_path))
    content = HEADER + "\n".join(chunks) + FOOTER
    ctx.actions.write(ctx.outputs.header, content)

fixtures_header = rule(
    implementation = _fixtures_header_impl,
    attrs = {
        "fixtures": attr.label_list(mandatory = True, allow_files = True),
        "header": attr.output(),
    },
)
