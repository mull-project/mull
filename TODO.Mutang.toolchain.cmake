# Make sure to delete CMake's CMakeCache.txt file or even the whole build folder.
# Before bootstrapping this file as a CMake toolchain.
# "CMake ignores toolchain file. How do get debug/error output?"
# http://stackoverflow.com/a/8303317/598057

set(CMAKE_SYSTEM_NAME Darwin)

set(LLVM_ROOT $ENV{LLVM_ROOT} CACHE PATH "Root of LLVM install.")

message(STATUS "LLVM_ROOT is ${LLVM_ROOT}")

# A bit of a sanity check:
if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
endif()

set(LLVM_PATH ${LLVM_ROOT} CACHE STRING "Native Client SDK Root Path")
message(STATUS "LLVM_PATH is ${LLVM_PATH}")

set(LLVM_HOST)
set(LLVM_TAG LLVM)

set(CMAKE_FIND_ROOT_PATH
	${LLVM_PATH}/include
	${LLVM_PATH}/usr/include
)
set(CMAKE_C_COMPILER   ${LLVM_PATH}/bin/clang)
set(CMAKE_CXX_COMPILER ${LLVM_PATH}/bin/clang++)
set(CMAKE_ASM_COMPILER ${LLVM_PATH}/bin/llvm-as)

# set(CMAKE_SYSROOT ???)
# set(CMAKE_STAGING_PREFIX ???)

# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

