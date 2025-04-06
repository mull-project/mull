# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@mull_package_info//:mull_package_info.bzl", "MULL_VERSION", "OS_ARCH", "OS_NAME", "OS_VERSION")
load("@rules_pkg//pkg:mappings.bzl", "pkg_attributes", "pkg_files")
load("@rules_pkg//pkg/private/deb:deb.bzl", "pkg_deb")
load("@rules_pkg//pkg/private/tar:tar.bzl", "pkg_tar")
load("@rules_pkg//pkg/private/zip:zip.bzl", "pkg_zip")
load("@rules_shell//shell:sh_binary.bzl", "sh_binary")
load(":mull_publish.bzl", "mull_publish_script")

def mull_package(name):
    package_names = []
    d = {}
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        # TODO: Add full LLVM version
        package_file_name = "Mull-{LLVM_VERSION}-{MULL_VERSION}-{OS_NAME}-{OS_ARCH}-{OS_VERSION}".format(
            LLVM_VERSION = llvm_version,
            MULL_VERSION = MULL_VERSION,
            OS_ARCH = OS_ARCH,
            OS_NAME = OS_NAME,
            OS_VERSION = OS_VERSION,
        )
        package_name = "mull-package-%s" % llvm_version
        package_names.append(package_name)
        d[llvm_version] = package_name

        if OS_NAME == "macOS":
            prefix = ""
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
                description = "Practical mutation testing and fault injection for C and C++",
                homepage = "https://github.com/mull-project/mull",
                license = "Apache-2.0",
                depends = [
                    "libclang-cpp%s" % llvm_version,
                    "libllvm%s" % llvm_version,
                    "libxml2",
                    "libsqlite3-0",
                ],
                package_file_name = "%s.deb" % package_file_name,
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
