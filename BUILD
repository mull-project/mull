load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_binary")
load("@rules_python//python:pip.bzl", "compile_pip_requirements")
load("//:mull_build.bzl", "mull_build")
load("//:mull_package.bzl", "mull_package")

package(default_visibility = ["//visibility:public"])

compile_pip_requirements(
    name = "requirements",
    src = "requirements.txt",
    requirements_darwin = "requirements_lock_macos.txt",
    requirements_linux = "requirements_lock_linux.txt",
    requirements_txt = "requirements_empty.txt",
)

py_binary(
    name = "cloudsmith_runner",
    srcs = ["cloudsmith_runner.py"],
    deps = [
        requirement("cloudsmith_cli"),
    ],
)

mull_build(name = "unused")

mull_package(name = "unused_2")
