# Make sure to delete CMake's CMakeCache.txt file or even the whole build folder.
# Before bootstrapping this file as a CMake toolchain.
# "CMake ignores toolchain file. How do get debug/error output?"
# http://stackoverflow.com/a/8303317/598057

MESSAGE(STATUS "Using Mull.toolchain.centos.cmake")

set(LLVM_ROOT "/opt/llvm-3.9.0" CACHE PATH "Root of LLVM install.")

set(CMAKE_C_COMPILER   ${LLVM_ROOT}/bin/clang)
set(CMAKE_CXX_COMPILER ${LLVM_ROOT}/bin/clang++)
set(CMAKE_ASM_COMPILER ${LLVM_ROOT}/bin/llvm-as)

