# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS", "EXACT_VERSION_MAPPING")
load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@mull_package_info//:mull_package_info.bzl", "MULL_DESCRIPTION", "MULL_HOMEPAGE", "MULL_VERSION", "OS_ARCH", "OS_NAME", "OS_VERSION")
load("@rules_pkg//pkg:mappings.bzl", "pkg_attributes", "pkg_files", "pkg_mklink")
load("@rules_pkg//pkg:rpm_pfg.bzl", "pkg_rpm")
load("@rules_pkg//pkg/private/deb:deb.bzl", "pkg_deb")
load("@rules_pkg//pkg/private/tar:tar.bzl", "pkg_tar")
load("@rules_pkg//pkg/private/zip:zip.bzl", "pkg_zip")
load("@rules_shell//shell:sh_binary.bzl", "sh_binary")
load("@rules_shell//shell:sh_test.bzl", "sh_test")
load(":mull_publish.bzl", "mull_publish_script")

# `mull`: the LLVM-independent tools
EXPECTED_MACOS_TOOLS_CONTENT = """bin/mull-reporter
bin/mull-runner
share/bash-completion/completions/mull-reporter
share/bash-completion/completions/mull-runner
share/fish/vendor_completions.d/mull-reporter.fish
share/fish/vendor_completions.d/mull-runner.fish
share/man/man1/mull-reporter.1
share/man/man1/mull-runner.1
share/zsh/site-functions/_mull-reporter
share/zsh/site-functions/_mull-runner
"""

EXPECTED_DEB_TOOLS_CONTENT = """usr/bin/mull-reporter
usr/bin/mull-runner
usr/share/bash-completion/completions/mull-reporter
usr/share/bash-completion/completions/mull-runner
usr/share/fish/vendor_completions.d/mull-reporter.fish
usr/share/fish/vendor_completions.d/mull-runner.fish
usr/share/man/man1/mull-reporter.1
usr/share/man/man1/mull-runner.1
usr/share/zsh/vendor-completions/_mull-reporter
usr/share/zsh/vendor-completions/_mull-runner
"""

EXPECTED_RPM_TOOLS_CONTENT = """/usr/bin/mull-reporter
/usr/bin/mull-runner
/usr/share/bash-completion/completions/mull-reporter
/usr/share/bash-completion/completions/mull-runner
/usr/share/fish/vendor_completions.d/mull-reporter.fish
/usr/share/fish/vendor_completions.d/mull-runner.fish
/usr/share/man/man1/mull-reporter.1
/usr/share/man/man1/mull-runner.1
/usr/share/zsh/site-functions/_mull-reporter
/usr/share/zsh/site-functions/_mull-runner
"""

# `mull-N`: the plugin and mull-instrument for LLVM N. On Linux it also keeps the
# old versioned tool names as symlinks; on macOS the Homebrew formula creates them.
EXPECTED_MACOS_LLVM_CONTENT = """bin/mull-instrument-{LLVM_VERSION}
lib/mull-ir-frontend-{LLVM_VERSION}
"""

EXPECTED_DEB_LLVM_CONTENT = """usr/bin/mull-instrument-{LLVM_VERSION}
usr/bin/mull-reporter-{LLVM_VERSION}
usr/bin/mull-runner-{LLVM_VERSION}
usr/lib/mull-ir-frontend-{LLVM_VERSION}
"""

EXPECTED_RPM_LLVM_CONTENT = """/usr/bin/mull-instrument-{LLVM_VERSION}
/usr/bin/mull-reporter-{LLVM_VERSION}
/usr/bin/mull-runner-{LLVM_VERSION}
/usr/lib/mull-ir-frontend-{LLVM_VERSION}
"""

def _package_contents_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name + ".txt")

    cmds = []
    if ctx.file.package.path.endswith(".zip"):
        cmds.append("unzip -l %s | awk 'NR > 3 { if ($0 ~ /----/) exit; print $NF }' | grep mull | LC_ALL=C sort > %s" % (ctx.file.package.path, out.path))
    elif ctx.file.package.path.endswith(".deb"):
        # Field 6 is the path; symlinks append "-> target"
        cmds.append("dpkg -c %s | awk '{ print $6 }' | sed 's|^\\./||' | grep mull | LC_ALL=C sort > %s" % (ctx.file.package.path, out.path))
    elif ctx.file.package.path.endswith(".rpm"):
        cmds.append("rpm -qlp %s | grep -v build-id | LC_ALL=C sort > %s" % (ctx.file.package.path, out.path))
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
    path = ctx.file.package.path
    if ctx.attr.llvm_version:
        template = EXPECTED_DEB_LLVM_CONTENT
        if path.endswith(".zip"):
            template = EXPECTED_MACOS_LLVM_CONTENT
        if path.endswith(".rpm"):
            template = EXPECTED_RPM_LLVM_CONTENT
    else:
        template = EXPECTED_DEB_TOOLS_CONTENT
        if path.endswith(".zip"):
            template = EXPECTED_MACOS_TOOLS_CONTENT
        if path.endswith(".rpm"):
            template = EXPECTED_RPM_TOOLS_CONTENT

    ctx.actions.write(out, template.format(LLVM_VERSION = ctx.attr.llvm_version))

    return [DefaultInfo(files = depset([out]))]

expected_package_contents = rule(
    implementation = _expected_package_contents_impl,
    attrs = {
        "package": attr.label(allow_single_file = True, mandatory = True),
        # Empty for the tools package
        "llvm_version": attr.string(),
    },
)

def _apt_dry_run_impl(ctx):
    out = ctx.actions.declare_file(ctx.attr.name)
    debs = " ".join(["./%s" % f.short_path for f in ctx.files.packages])
    ctx.actions.write(out, "apt-get install --dry-run %s" % debs)
    return [DefaultInfo(files = depset([out]))]

apt_dry_run_sh = rule(
    implementation = _apt_dry_run_impl,
    attrs = {
        # The LLVM packages depend on the tools package, so install them together
        "packages": attr.label_list(allow_files = [".deb"], mandatory = True),
    },
)

def _package_tests(name, package, llvm_version = ""):
    package_contents(
        name = "%s-content" % name,
        package = package,
    )
    expected_package_contents(
        name = "%s-expected-content" % name,
        package = package,
        llvm_version = llvm_version,
    )
    diff_test(
        name = "%s_contents_test" % name,
        file1 = "%s-expected-content" % name,
        file2 = "%s-content" % name,
        size = "small",
    )

def _apt_test(name, packages):
    apt_dry_run_sh(
        name = name + ".sh",
        packages = packages,
    )
    sh_test(
        name = name + "_apt_test",
        srcs = [name + ".sh"],
        data = packages,
        deps = ["@bazel_tools//tools/bash/runfiles"],
    )

def _mull_tools_package(prefix):
    package_name = "mull-tools-package"
    package_file_name = "Mull-{MULL_VERSION}-{OS_NAME}-{OS_ARCH}-{OS_VERSION}".format(
        MULL_VERSION = MULL_VERSION,
        OS_ARCH = OS_ARCH,
        OS_NAME = OS_NAME,
        OS_VERSION = OS_VERSION,
    )

    pkg_files(
        name = "%s-binaries" % package_name,
        srcs = [
            "//rust/mull-tools:mull-reporter",
            "//rust/mull-tools:mull-runner",
        ],
        prefix = "%sbin" % prefix,
        attributes = pkg_attributes(mode = "755"),
    )
    pkg_files(
        name = "%s-manpages" % package_name,
        srcs = [
            "//rust/mull-docs:mull-runner.1",
            "//rust/mull-docs:mull-reporter.1",
        ],
        prefix = "%sshare/man/man1" % prefix,
    )
    pkg_files(
        name = "%s-bash-completions" % package_name,
        srcs = [
            "//rust/mull-docs:mull-runner.bash",
            "//rust/mull-docs:mull-reporter.bash",
        ],
        prefix = "%sshare/bash-completion/completions" % prefix,
        renames = {
            "//rust/mull-docs:mull-runner.bash": "mull-runner",
            "//rust/mull-docs:mull-reporter.bash": "mull-reporter",
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
            "//rust/mull-docs:mull-runner.zsh",
            "//rust/mull-docs:mull-reporter.zsh",
        ],
        prefix = zsh_prefix,
        renames = {
            "//rust/mull-docs:mull-runner.zsh": "_mull-runner",
            "//rust/mull-docs:mull-reporter.zsh": "_mull-reporter",
        },
    )
    pkg_files(
        name = "%s-fish-completions" % package_name,
        srcs = [
            "//rust/mull-docs:mull-runner.fish",
            "//rust/mull-docs:mull-reporter.fish",
        ],
        prefix = "%sshare/fish/vendor_completions.d" % prefix,
    )

    srcs = [
        "%s-binaries" % package_name,
        "%s-manpages" % package_name,
        "%s-bash-completions" % package_name,
        "%s-zsh-completions" % package_name,
        "%s-fish-completions" % package_name,
    ]
    if OS_NAME == "macOS":
        pkg_zip(
            name = package_name,
            srcs = srcs,
            out = "%s.zip" % package_file_name,
            stamp = -1,
        )
    elif OS_NAME == "rhel":
        pkg_rpm(
            name = package_name,
            srcs = srcs,
            version = MULL_VERSION,
            summary = MULL_DESCRIPTION,
            description = MULL_DESCRIPTION,
            url = MULL_HOMEPAGE,
            license = "Apache-2.0",
            package_name = "mull",
            release = "1",
            package_file_name = "%s.rpm" % package_file_name,
            defines = {"__brp_compress": "%{nil}"},
        )
    else:
        pkg_tar(
            name = "%s-tar" % package_name,
            srcs = srcs,
            stamp = -1,
        )
        pkg_deb(
            name = package_name,
            data = "%s-tar" % package_name,
            maintainer = "Alex Denisov <alex@lowlevelbits.org>",
            package = "mull",
            version = MULL_VERSION,
            description = MULL_DESCRIPTION,
            homepage = MULL_HOMEPAGE,
            license = "Apache-2.0",
            package_file_name = "%s.deb" % package_file_name,
            architecture = "arm64" if OS_ARCH == "aarch64" else OS_ARCH,
        )
        _apt_test(package_name, [package_name])

    _package_tests(package_name, package_name)
    return package_name

def _mull_llvm_package(prefix, llvm_version, tools_package):
    package_name = "mull-package-%s" % llvm_version
    package_file_name = "Mull-{LLVM_VERSION}-{MULL_VERSION}-LLVM-{FULL_LLVM_VERSION}-{OS_NAME}-{OS_ARCH}-{OS_VERSION}".format(
        LLVM_VERSION = llvm_version,
        MULL_VERSION = MULL_VERSION,
        OS_ARCH = OS_ARCH,
        OS_NAME = OS_NAME,
        OS_VERSION = OS_VERSION,
        FULL_LLVM_VERSION = EXACT_VERSION_MAPPING[llvm_version],
    )

    pkg_files(
        name = "%s-binaries" % package_name,
        srcs = ["//:mull-instrument-%s" % llvm_version],
        prefix = "%sbin" % prefix,
        attributes = pkg_attributes(mode = "755"),
    )
    pkg_files(
        name = "%s-libraries" % package_name,
        srcs = ["//:mull-ir-frontend-%s-gen" % llvm_version],
        prefix = "%slib" % prefix,
        attributes = pkg_attributes(mode = "755"),
    )
    srcs = [
        "%s-binaries" % package_name,
        "%s-libraries" % package_name,
    ]

    # TODO: drop the versioned names once users had time to move to mull-runner/mull-reporter
    if OS_NAME != "macOS":
        for tool in ["mull-runner", "mull-reporter"]:
            pkg_mklink(
                name = "%s-%s-link" % (package_name, tool),
                link_name = "%sbin/%s-%s" % (prefix, tool, llvm_version),
                target = tool,
            )
            srcs.append("%s-%s-link" % (package_name, tool))

    if OS_NAME == "macOS":
        pkg_zip(
            name = package_name,
            srcs = srcs,
            out = "%s.zip" % package_file_name,
            stamp = -1,
        )
    elif OS_NAME == "rhel":
        pkg_rpm(
            name = package_name,
            srcs = srcs,
            version = MULL_VERSION,
            summary = MULL_DESCRIPTION,
            description = MULL_DESCRIPTION,
            url = MULL_HOMEPAGE,
            license = "Apache-2.0",
            requires = [
                "clang-libs",
                "llvm-libs",
                "mull = %s" % MULL_VERSION,
            ],
            package_name = "mull-%s" % llvm_version,
            release = "1",
            package_file_name = "%s.rpm" % package_file_name,
            defines = {"__brp_compress": "%{nil}"},
        )
    else:
        pkg_tar(
            name = "%s-tar" % package_name,
            srcs = srcs,
            stamp = -1,
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
                "mull (= %s)" % MULL_VERSION,
            ],
            package_file_name = "%s.deb" % package_file_name,
            architecture = "arm64" if OS_ARCH == "aarch64" else OS_ARCH,
        )
        _apt_test(package_name, [tools_package, package_name])

    _package_tests(package_name, package_name, llvm_version)
    return package_name

def mull_package(name):
    if OS_NAME == "macOS":
        prefix = ""  # Homebrew manages its own prefix
    else:
        prefix = "/usr/"

    tools_package = _mull_tools_package(prefix)

    llvm_packages = {}
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        llvm_packages["mull-%s" % llvm_version] = _mull_llvm_package(prefix, llvm_version, tools_package)

    native.filegroup(
        name = "mull_tools_packages",
        srcs = [tools_package],
    )
    native.filegroup(
        name = "mull_llvm_packages",
        srcs = llvm_packages.values(),
    )
    native.filegroup(
        name = "mull_packages",
        srcs = [tools_package] + llvm_packages.values(),
    )

    mull_publish_script(
        name = "publish_tools.sh",
        packages = {"mull": tools_package},
    )
    sh_binary(
        name = "publish_tools",
        srcs = [":publish_tools.sh"],
        data = ["//:cloudsmith_runner", tools_package],
    )
    mull_publish_script(
        name = "publish.sh",
        packages = llvm_packages,
    )
    sh_binary(
        name = "publish",
        srcs = [":publish.sh"],
        data = ["//:cloudsmith_runner"] + llvm_packages.values(),
    )
