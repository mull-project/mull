# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS", "EXACT_VERSION_MAPPING")
load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@mull_package_info//:mull_package_info.bzl", "MULL_DESCRIPTION", "MULL_HOMEPAGE", "MULL_VERSION", "OS_ARCH", "OS_NAME", "OS_VERSION")
load("@rules_pkg//pkg:mappings.bzl", "pkg_attributes", "pkg_files")
load("@rules_pkg//pkg/private/deb:deb.bzl", "pkg_deb")
load("@rules_pkg//pkg/private/tar:tar.bzl", "pkg_tar")
load("@rules_pkg//pkg/private/zip:zip.bzl", "pkg_zip")
load("@rules_shell//shell:sh_binary.bzl", "sh_binary")
load(":mull_publish.bzl", "mull_publish_script")

EXPECTED_MACOS_PACKAGE_CONTENT = """usr/
usr/local/
usr/local/bin/
usr/local/bin/mull-reporter-{LLVM_VERSION}
usr/local/bin/mull-runner-{LLVM_VERSION}
usr/local/lib/
usr/local/lib/mull-ir-frontend-{LLVM_VERSION}
"""

EXPECTED_LINUX_PACKAGE_CONTENT = """usr/
usr/bin/
usr/bin/mull-reporter-{LLVM_VERSION}
usr/bin/mull-runner-{LLVM_VERSION}
usr/lib/
usr/lib/mull-ir-frontend-{LLVM_VERSION}
"""

def _package_contents_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name + ".txt")

    cmds = []
    if ctx.file.package.path.endswith(".zip"):
        cmds.append("unzip -l %s | awk 'NR > 3 { if ($0 ~ /----/) exit; print $NF }' > %s" % (ctx.file.package.path, out.path))
    elif ctx.file.package.path.endswith(".deb"):
        cmds.append("dpkg -c %s | awk '{ print $NF }' > %s" % (ctx.file.package.path, out.path))
    else:
        fail("Unsupported package format: %s" % ctx.file.package.path)

    script = ctx.actions.declare_file(ctx.attr.name + ".sh")
    ctx.actions.write(script, "\n".join(cmds), is_executable = True)

    ctx.actions.run_shell(
        inputs = [ctx.file.package, script],
        outputs = [out],
        command = script.path,
    )

    return [DefaultInfo(files = depset([out]))]

package_contents = rule(
    implementation = _package_contents_impl,
    attrs = {
        "package": attr.label(allow_single_file = True, mandatory = True),
    },
)

def _expected_package_contents_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name + ".txt")
    template = EXPECTED_LINUX_PACKAGE_CONTENT
    if ctx.file.package.path.endswith(".zip"):
        template = EXPECTED_MACOS_PACKAGE_CONTENT

    ctx.actions.write(out, template.format(LLVM_VERSION = ctx.attr.llvm_version))

    return [DefaultInfo(files = depset([out]))]

expected_package_contents = rule(
    implementation = _expected_package_contents_impl,
    attrs = {
        "package": attr.label(allow_single_file = True, mandatory = True),
        "llvm_version": attr.string(mandatory = True),
    },
)

def mull_package(name):
    package_names = []
    d = {}
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        package_file_name = "Mull-{LLVM_VERSION}-{MULL_VERSION}-LLVM-{FULL_LLVM_VERSION}-{OS_NAME}-{OS_ARCH}-{OS_VERSION}".format(
            LLVM_VERSION = llvm_version,
            MULL_VERSION = MULL_VERSION,
            OS_ARCH = OS_ARCH,
            OS_NAME = OS_NAME,
            OS_VERSION = OS_VERSION,
            FULL_LLVM_VERSION = EXACT_VERSION_MAPPING[llvm_version],
        )
        package_name = "mull-package-%s" % llvm_version
        package_names.append(package_name)
        d[llvm_version] = package_name

        if OS_NAME == "macOS":
            prefix = "/usr/local/"
        else:
            prefix = "/usr/"

        pkg_files(
            name = "%s-binaries" % package_name,
            srcs = [
                "//:mull-reporter-%s" % llvm_version,
                "//:mull-runner-%s" % llvm_version,
            ],
            prefix = "%sbin" % prefix,
            attributes = pkg_attributes(mode = "755"),
        )
        pkg_files(
            name = "%s-libraries" % package_name,
            srcs = [
                "//:mull-ir-frontend-%s-gen" % llvm_version,
            ],
            prefix = "%slib" % prefix,
            attributes = pkg_attributes(mode = "755"),
        )

        if OS_NAME == "macOS":
            pkg_zip(
                name = package_name,
                srcs = [
                    "%s-binaries" % package_name,
                    "%s-libraries" % package_name,
                ],
                stamp = 1,
                out = "%s.zip" % package_file_name,
            )
        else:
            pkg_tar(
                name = "%s-tar" % package_name,
                srcs = [
                    "%s-binaries" % package_name,
                    "%s-libraries" % package_name,
                ],
                stamp = 1,
            )
            pkg_deb(
                name = package_name,
                data = "%s-tar" % package_name,
                maintainer = "Alex Denisov <alex@lowlevelbits.org>",
                package = "mull-%s" % llvm_version,
                version = MULL_VERSION,
                description = MULL_DESCRIPTION,
                homepage = MULL_HOMEPAGE,
                license = "Apache-2.0",
                depends = [
                    "libclang-cpp%s" % llvm_version,
                    "libllvm%s" % llvm_version,
                    "libxml2",
                    "libsqlite3-0",
                ],
                package_file_name = "%s.deb" % package_file_name,
            )

        package_contents(
            name = "mull-package-content-%s" % llvm_version,
            package = package_name,
        )
        expected_package_contents(
            name = "mull-expected-package-content-%s" % llvm_version,
            package = package_name,
            llvm_version = llvm_version,
        )
        diff_test(
            name = "package_contents_%s_test" % llvm_version,
            file1 = "mull-expected-package-content-%s" % llvm_version,
            file2 = "mull-package-content-%s" % llvm_version,
            size = "small",
        )

    native.filegroup(
        name = "mull_packages",
        srcs = package_names,
    )
    mull_publish_script(
        name = "publish.sh",
        packages = d,
    )
    sh_binary(
        name = "publish",
        srcs = [":publish.sh"],
        data = [
            "//:cloudsmith_runner",
        ] + package_names,
    )
