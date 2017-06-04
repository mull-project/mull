# Make sure to delete CMake's CMakeCache.txt file or even the whole build folder.
# Before bootstrapping this file as a CMake toolchain.
# "CMake ignores toolchain file. How do get debug/error output?"
# http://stackoverflow.com/a/8303317/598057

MESSAGE(STATUS "Using Mull.toolchain.OSX.cmake")

set(LLVM_ROOT "/usr/lib/llvm-3.9" CACHE PATH "Root of LLVM install.")

# A bit of a sanity check:
if (NOT EXISTS ${LLVM_ROOT}/include/llvm)
message(SEND_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
endif()

set(MULL_TOOLCHAIN_SQLITE_PATH "/usr/lib/x86_64-linux-gnu")
set(MULL_TOOLCHAIN_ZLIB_PATH "/usr/lib/x86_64-linux-gnu")

set(CMAKE_C_COMPILER   ${LLVM_ROOT}/bin/clang)
set(CMAKE_CXX_COMPILER ${LLVM_ROOT}/bin/clang++)
set(CMAKE_ASM_COMPILER ${LLVM_ROOT}/bin/llvm-as)

