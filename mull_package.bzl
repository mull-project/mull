# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS", "EXACT_VERSION_MAPPING")
load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@mull_package_info//:mull_package_info.bzl", "MULL_DESCRIPTION", "MULL_HOMEPAGE", "MULL_VERSION", "OS_ARCH", "OS_NAME", "OS_VERSION")
load("@rules_pkg//pkg:mappings.bzl", "pkg_attributes", "pkg_files")
load("@rules_pkg//pkg:rpm_pfg.bzl", "pkg_rpm")
load("@rules_pkg//pkg/private/deb:deb.bzl", "pkg_deb")
load("@rules_pkg//pkg/private/tar:tar.bzl", "pkg_tar")
load("@rules_pkg//pkg/private/zip:zip.bzl", "pkg_zip")
load("@rules_shell//shell:sh_binary.bzl", "sh_binary")
load("@rules_shell//shell:sh_test.bzl", "sh_test")
load(":mull_publish.bzl", "mull_publish_script")

EXPECTED_MACOS_PACKAGE_CONTENT = """usr/local/bin/mull-reporter-{LLVM_VERSION}
usr/local/bin/mull-runner-{LLVM_VERSION}
usr/local/lib/mull-ir-frontend-{LLVM_VERSION}
usr/local/share/bash-completion/completions/mull-reporter-{LLVM_VERSION}
usr/local/share/bash-completion/completions/mull-runner-{LLVM_VERSION}
usr/local/share/fish/vendor_completions.d/mull-reporter-{LLVM_VERSION}.fish
usr/local/share/fish/vendor_completions.d/mull-runner-{LLVM_VERSION}.fish
usr/local/share/man/man1/mull-reporter-{LLVM_VERSION}.1
usr/local/share/man/man1/mull-runner-{LLVM_VERSION}.1
usr/local/share/zsh/site-functions/_mull-reporter-{LLVM_VERSION}
usr/local/share/zsh/site-functions/_mull-runner-{LLVM_VERSION}
"""

EXPECTED_DEB_PACKAGE_CONTENT = """usr/bin/mull-reporter-{LLVM_VERSION}
usr/bin/mull-runner-{LLVM_VERSION}
usr/lib/mull-ir-frontend-{LLVM_VERSION}
usr/share/bash-completion/completions/mull-reporter-{LLVM_VERSION}
usr/share/bash-completion/completions/mull-runner-{LLVM_VERSION}
usr/share/fish/vendor_completions.d/mull-reporter-{LLVM_VERSION}.fish
usr/share/fish/vendor_completions.d/mull-runner-{LLVM_VERSION}.fish
usr/share/man/man1/mull-reporter-{LLVM_VERSION}.1
usr/share/man/man1/mull-runner-{LLVM_VERSION}.1
usr/share/zsh/vendor-completions/_mull-reporter-{LLVM_VERSION}
usr/share/zsh/vendor-completions/_mull-runner-{LLVM_VERSION}
"""

EXPECTED_RPM_PACKAGE_CONTENT = """/usr/bin/mull-reporter-{LLVM_VERSION}
/usr/bin/mull-runner-{LLVM_VERSION}
/usr/lib/mull-ir-frontend-{LLVM_VERSION}
/usr/share/bash-completion/completions/mull-reporter-{LLVM_VERSION}
/usr/share/bash-completion/completions/mull-runner-{LLVM_VERSION}
/usr/share/fish/vendor_completions.d/mull-reporter-{LLVM_VERSION}.fish
/usr/share/fish/vendor_completions.d/mull-runner-{LLVM_VERSION}.fish
/usr/share/man/man1/mull-reporter-{LLVM_VERSION}.1
/usr/share/man/man1/mull-runner-{LLVM_VERSION}.1
/usr/share/zsh/site-functions/_mull-reporter-{LLVM_VERSION}
/usr/share/zsh/site-functions/_mull-runner-{LLVM_VERSION}
"""

def _package_contents_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name + ".txt")

    cmds = []
    if ctx.file.package.path.endswith(".zip"):
        cmds.append("unzip -l %s | awk 'NR > 3 { if ($0 ~ /----/) exit; print $NF }' | grep mull > %s" % (ctx.file.package.path, out.path))
    elif ctx.file.package.path.endswith(".deb"):
        cmds.append("dpkg -c %s | awk '{ print $NF }' | grep mull > %s" % (ctx.file.package.path, out.path))
    elif ctx.file.package.path.endswith(".rpm"):
        cmds.append("rpm -qlp %s | grep -v build-id > %s" % (ctx.file.package.path, out.path))
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
    template = EXPECTED_DEB_PACKAGE_CONTENT
    if ctx.file.package.path.endswith(".zip"):
        template = EXPECTED_MACOS_PACKAGE_CONTENT
    if ctx.file.package.path.endswith(".rpm"):
        template = EXPECTED_RPM_PACKAGE_CONTENT

    ctx.actions.write(out, template.format(LLVM_VERSION = ctx.attr.llvm_version))

    return [DefaultInfo(files = depset([out]))]

expected_package_contents = rule(
    implementation = _expected_package_contents_impl,
    attrs = {
        "package": attr.label(allow_single_file = True, mandatory = True),
        "llvm_version": attr.string(mandatory = True),
    },
)

def _apt_dry_run_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name)
    ctx.actions.write(out, "apt-get install --dry-run ./%s" % ctx.file.package.short_path)
    return [DefaultInfo(files = depset([out]))]

apt_dry_run_sh = rule(
    implementation = _apt_dry_run_impl,
    attrs = {
        "package": attr.label(allow_single_file = True, mandatory = True),
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
        pkg_files(
            name = "%s-manpages" % package_name,
            srcs = [
                "//rust/mull-docs:mull-runner.1",
                "//rust/mull-docs:mull-reporter.1",
            ],
            prefix = "%sshare/man/man1" % prefix,
            renames = {
                "//rust/mull-docs:mull-runner.1": "mull-runner-%s.1" % llvm_version,
                "//rust/mull-docs:mull-reporter.1": "mull-reporter-%s.1" % llvm_version,
            },
        )

        # Generate versioned completions: replace command names in the content
        # (e.g. "mull-runner" -> "mull-runner-19") so completions match the
        # version-suffixed binary names.
        native.genrule(
            name = "%s-completions-gen" % package_name,
            srcs = [
                "//rust/mull-docs:mull-runner.bash",
                "//rust/mull-docs:mull-reporter.bash",
                "//rust/mull-docs:mull-runner.zsh",
                "//rust/mull-docs:mull-reporter.zsh",
                "//rust/mull-docs:mull-runner.fish",
                "//rust/mull-docs:mull-reporter.fish",
            ],
            outs = [
                "%s-mull-runner.bash" % package_name,
                "%s-mull-reporter.bash" % package_name,
                "%s-mull-runner.zsh" % package_name,
                "%s-mull-reporter.zsh" % package_name,
                "%s-mull-runner.fish" % package_name,
                "%s-mull-reporter.fish" % package_name,
            ],
            cmd = " && ".join([
                "sed 's/mull-runner/mull-runner-%s/g' $(location //rust/mull-docs:mull-runner.bash) > $(location %s-mull-runner.bash)" % (llvm_version, package_name),
                "sed 's/mull-reporter/mull-reporter-%s/g' $(location //rust/mull-docs:mull-reporter.bash) > $(location %s-mull-reporter.bash)" % (llvm_version, package_name),
                "sed 's/mull-runner/mull-runner-%s/g' $(location //rust/mull-docs:mull-runner.zsh) > $(location %s-mull-runner.zsh)" % (llvm_version, package_name),
                "sed 's/mull-reporter/mull-reporter-%s/g' $(location //rust/mull-docs:mull-reporter.zsh) > $(location %s-mull-reporter.zsh)" % (llvm_version, package_name),
                "sed 's/mull-runner/mull-runner-%s/g' $(location //rust/mull-docs:mull-runner.fish) > $(location %s-mull-runner.fish)" % (llvm_version, package_name),
                "sed 's/mull-reporter/mull-reporter-%s/g' $(location //rust/mull-docs:mull-reporter.fish) > $(location %s-mull-reporter.fish)" % (llvm_version, package_name),
            ]),
        )
        pkg_files(
            name = "%s-bash-completions" % package_name,
            srcs = [
                "%s-mull-runner.bash" % package_name,
                "%s-mull-reporter.bash" % package_name,
            ],
            prefix = "%sshare/bash-completion/completions" % prefix,
            renames = {
                "%s-mull-runner.bash" % package_name: "mull-runner-%s" % llvm_version,
                "%s-mull-reporter.bash" % package_name: "mull-reporter-%s" % llvm_version,
            },
        )

        # Deb uses vendor-completions, macOS/RPM use site-functions
        if OS_NAME == "ubuntu" or OS_NAME == "debian":
            zsh_prefix = "%sshare/zsh/vendor-completions" % prefix
        else:
            zsh_prefix = "%sshare/zsh/site-functions" % prefix
        pkg_files(
            name = "%s-zsh-completions" % package_name,
            srcs = [
                "%s-mull-runner.zsh" % package_name,
                "%s-mull-reporter.zsh" % package_name,
            ],
            prefix = zsh_prefix,
            renames = {
                "%s-mull-runner.zsh" % package_name: "_mull-runner-%s" % llvm_version,
                "%s-mull-reporter.zsh" % package_name: "_mull-reporter-%s" % llvm_version,
            },
        )
        pkg_files(
            name = "%s-fish-completions" % package_name,
            srcs = [
                "%s-mull-runner.fish" % package_name,
                "%s-mull-reporter.fish" % package_name,
            ],
            prefix = "%sshare/fish/vendor_completions.d" % prefix,
            renames = {
                "%s-mull-runner.fish" % package_name: "mull-runner-%s.fish" % llvm_version,
                "%s-mull-reporter.fish" % package_name: "mull-reporter-%s.fish" % llvm_version,
            },
        )

        if OS_NAME == "macOS":
            pkg_zip(
                name = package_name,
                srcs = [
                    "%s-binaries" % package_name,
                    "%s-libraries" % package_name,
                    "%s-manpages" % package_name,
                    "%s-bash-completions" % package_name,
                    "%s-zsh-completions" % package_name,
                    "%s-fish-completions" % package_name,
                ],
                stamp = 1,
                out = "%s.zip" % package_file_name,
            )
        elif OS_NAME == "rhel":
            pkg_rpm(
                name = package_name,
                srcs = [
                    "%s-binaries" % package_name,
                    "%s-libraries" % package_name,
                    "%s-manpages" % package_name,
                    "%s-bash-completions" % package_name,
                    "%s-zsh-completions" % package_name,
                    "%s-fish-completions" % package_name,
                ],
                version = MULL_VERSION,
                summary = MULL_DESCRIPTION,
                description = MULL_DESCRIPTION,
                url = MULL_HOMEPAGE,
                license = "Apache-2.0",
                requires = [
                    "clang-libs",
                    "llvm-libs",
                    "libxml2",
                ],
                package_name = "mull-%s" % llvm_version,
                release = "1",
                package_file_name = "%s.rpm" % package_file_name,
                defines = {"__brp_compress": "%{nil}"},
            )
        else:
            pkg_tar(
                name = "%s-tar" % package_name,
                srcs = [
                    "%s-binaries" % package_name,
                    "%s-libraries" % package_name,
                    "%s-manpages" % package_name,
                    "%s-bash-completions" % package_name,
                    "%s-zsh-completions" % package_name,
                    "%s-fish-completions" % package_name,
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
                architecture = "arm64" if OS_ARCH == "aarch64" else OS_ARCH,
            )
            apt_dry_run_sh(
                name = package_name + ".sh",
                package = package_name,
            )
            sh_test(
                name = package_name + "_apt_test",
                srcs = [package_name + ".sh"],
                data = [package_name],
                deps = ["@bazel_tools//tools/bash/runfiles"],
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
