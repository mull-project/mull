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

print(r.Rlocation("mull/tests/integration/filecheck_runner"))
print(r.Rlocation("mull/mull-ir-frontend-18"))

# LLVM LIT: Is it possible to pass user defined parameter into a test script?
# https://stackoverflow.com/a/39861848/598057
# current_dir = os.environ.get("CURRENT_DIR", "")
clang_cc = r.Rlocation("llvm_18/clang")
clang_cxx = r.Rlocation("llvm_18/clangxx")
llvm_profdata = r.Rlocation("llvm_18/llvm-profdata")
mull_runner = r.Rlocation("mull/mull_runner_18")
mull_reporter = r.Rlocation("mull/mull_reporter_18")
mull_frontend_cxx = r.Rlocation("mull/mull-ast-frontend-18")
mull_ir_frontend = r.Rlocation("mull/mull-ir-frontend-18")
filecheck = r.Rlocation("mull/tests/integration/filecheck_runner")
llvm_major_version = os.environ.get("LLVM_VERSION_MAJOR", "")
test_cxx_flags = "-Wall"  # os.environ.get("TEST_CXX_FLAGS", "")
python3 = os.environ.get("python3", "")

assert llvm_major_version
assert test_cxx_flags

test_cxx_flags = unescape_string(test_cxx_flags)

# config.substitutions.append(("%CURRENT_DIR", current_dir))
config.substitutions.append(("%clang_cc", clang_cc))
config.substitutions.append(("%clang_cxx", clang_cxx))
config.substitutions.append(("%sysroot", os.environ.get("sysroot", "")))
config.substitutions.append(("%llvm_profdata", llvm_profdata))
config.substitutions.append(("%mull_runner", mull_runner))
config.substitutions.append(("%mull_reporter", mull_reporter))
config.substitutions.append(("%mull_frontend_cxx", mull_frontend_cxx))
config.substitutions.append(("%mull_ir_frontend", mull_ir_frontend))
config.substitutions.append(("%filecheck", filecheck))
config.substitutions.append(("%TEST_CXX_FLAGS", test_cxx_flags))
config.substitutions.append(("%python3", python3))

config.suffixes = [".cpp", ".c", ".itest"]

if platform.system() == "Darwin":
    config.available_features.add("MACOS")
else:
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
