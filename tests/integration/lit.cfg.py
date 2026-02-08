import platform
import re
import os
import lit.formats

from python.runfiles import Runfiles

r = Runfiles.Create()


# The paths passed by CMake as env variables to LIT get escaped so we have to
# undo it to pass it down to the LIT tests.
# Example: "-isystem\ /opt/llvm-3.9.0/include/c++/v1 ..."
def unescape_string(string):
    return re.sub(r"\\ ", " ", string)


def get_os_filename_string():
    if platform.system() == "Darwin":
        return "macOS"
    if platform.system() == "Linux":
        return "Linux"
    print("error: lit.cfg could not detect OS")
    return "Unknown"


config = globals()["config"]
config.name = "Mull integration tests"
config.test_format = lit.formats.ShTest("0")

llvm_major_version = os.environ.get("LLVM_VERSION_MAJOR", None)

assert llvm_major_version

clang_cc = r.Rlocation(f"llvm_{llvm_major_version}/clang")
clang_cxx = r.Rlocation(f"llvm_{llvm_major_version}/clangxx")
llvm_profdata = r.Rlocation(f"llvm_{llvm_major_version}/llvm-profdata")
mull_runner = r.Rlocation(f"mull/mull-runner-{llvm_major_version}")
mull_reporter = r.Rlocation(f"_main/rust/mull-tools/mull-reporter-{llvm_major_version}")
mull_frontend_cxx = r.Rlocation(f"mull/mull-ast-frontend-{llvm_major_version}")
mull_ir_frontend = r.Rlocation(f"mull/mull-ir-frontend-{llvm_major_version}")
filecheck = r.Rlocation("mull/tests/integration/filecheck_runner")

python3 = r.Rlocation("mull/tests/integration/python3")

config.substitutions.append(("%clang_cc", clang_cc))
config.substitutions.append(("%clang_cxx", clang_cxx))
config.substitutions.append(("%sysroot", os.environ.get("sysroot", "")))
config.substitutions.append(("%llvm_profdata", llvm_profdata))
config.substitutions.append(("%mull_runner", mull_runner))
config.substitutions.append(("%mull_reporter", mull_reporter))
config.substitutions.append(("%mull_frontend_cxx", mull_frontend_cxx))
config.substitutions.append(("%mull_ir_frontend", mull_ir_frontend))
config.substitutions.append(("%filecheck", filecheck))
config.substitutions.append(("%python3", python3))

config.suffixes = [".cpp", ".c", ".itest"]

if platform.system() == "Darwin":
    config.available_features.add("MACOS")
    config.substitutions.append(("%libcxx", "-stdlib=libc++ -lc++"))
else:
    config.substitutions.append(("%libcxx", "-stdlib=libstdc++ -lstdc++"))
    config.available_features.add("LINUX")

if int(llvm_major_version) >= 16:
    config.substitutions.append(
        ("%pass_mull_ir_frontend", "-fpass-plugin=" + mull_ir_frontend)
    )
else:
    config.substitutions.append(
        (
            "%pass_mull_ir_frontend",
            "-fexperimental-new-pass-manager -fpass-plugin=" + mull_ir_frontend,
        )
    )
