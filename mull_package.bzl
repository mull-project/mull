# buildifier: disable=module-docstring
load("@available_llvm_versions//:mull_llvm_versions.bzl", "AVAILABLE_LLVM_VERSIONS")
load("@mull_package_info//:mull_package_info.bzl", "MULL_VERSION", "OS_ARCH", "OS_NAME", "OS_VERSION")
load("@rules_pkg//pkg:mappings.bzl", "pkg_attributes", "pkg_files")
load("@rules_pkg//pkg/private/zip:zip.bzl", "pkg_zip")

def mull_package(name):
    package_names = []
    for llvm_version in AVAILABLE_LLVM_VERSIONS:
        # TODO: Add full LLVM version
        package_name = "Mull-{LLVM_VERSION}-{MULL_VERSION}-{OS_NAME}-{OS_ARCH}-{OS_VERSION}".format(
            LLVM_VERSION = llvm_version,
            MULL_VERSION = MULL_VERSION,
            OS_ARCH = OS_ARCH,
            OS_NAME = OS_NAME,
            OS_VERSION = OS_VERSION,
        )
        package_names.append(package_name)

        pkg_files(
            name = "%s-binaries" % package_name,
            srcs = [
                "//:mull-reporter-%s" % llvm_version,
                "//:mull-runner-%s" % llvm_version,
            ],
            prefix = "bin",
            attributes = pkg_attributes(mode = "755"),
        )
        pkg_files(
            name = "%s-libraries" % package_name,
            srcs = [
                "//:mull-ir-frontend-%s-gen" % llvm_version,
            ],
            prefix = "lib",
            attributes = pkg_attributes(mode = "755"),
        )

        pkg_zip(
            name = package_name,
            srcs = [
                "%s-binaries" % package_name,
                "%s-libraries" % package_name,
            ],
            stamp = 1,
        )
    native.filegroup(
        name = "mull_packages",
        srcs = package_names,
    )
