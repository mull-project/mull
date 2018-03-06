cmake_minimum_required(VERSION 3.5.1)

project(Mull)
enable_language(C CXX)

# This enables assertions for Release builds.
# https://stackoverflow.com/questions/22140520/how-to-enable-assert-in-cmake-release-mode
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Setting THREADS_PREFER_PTHREAD_FLAG and finding package Threads resolves:
# /usr/bin/ld: ../../gtest-1.7.0/libgtest.a(gtest-all.cc.o): undefined reference to symbol     'pthread_key_delete@@GLIBC_2.2.5'
# /lib/x86_64-linux-gnu/libpthread.so.0: error adding symbols: DSO missing from command line
# https://stackoverflow.com/questions/25617839/undefined-reference-to-symbol-pthread-key-deleteglibc-2-2-5
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

if (NOT CMAKE_TOOLCHAIN_FILE)
message(FATAL_ERROR "This CMakeLists.txt file expects cmake to be called with a CMake toolchain file:
cmake ... -DCMAKE_TOOLCHAIN_FILE=path_to_your_toolchain_file.
See example: Mull.toolchain.OSX.cmake")
endif()

if (NOT LLVM_ROOT)
  message(FATAL_ERROR "This CMakeLists.txt file expects cmake to be called with a
    CMake toolchain file which provides LLVM_ROOT:\n \
    -DCMAKE_TOOLCHAIN_FILE=path_to_your_toolchain")
endif()

include(Mull.LLVM.cmake)

## LLVM CXX flags
llvm_get_cxx_flags(LLVM_CXX_FLAGS)

## LLVM lib directory
llvm_get_lib_dir(LLVM_LIB_PATH)

## LLVM linker flags.
llvm_get_link_flags(LLVM_LINK_FLAGS)
# Without -undefined,dynamic_lookup flags there is a problem with order in which
# the libraries are linked and that causes missing symbols.
# https://github.com/eliben/llvm-clang-samples/pull/11
set(LLVM_LINK_FLAGS "${LLVM_LINK_FLAGS} -Wl,-undefined,dynamic_lookup")

# LLVM libraries
set(llvm_components
  "asmparser"
  "orcjit"
  "support"
  "x86")
llvm_get_libs(MULL_DEPENDENCY_LLVM_LIBRARIES "${llvm_components}")

# https://cmake.org/Wiki/CMake_RPATH_handling#Always_full_RPATH
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" "${LLVM_LIB_PATH}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(MULL_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MULL_INCLUDE_DIR  ${MULL_SOURCE_DIR}/include)

include_directories("${LLVM_ROOT}/include")

## LLVM's dependencies.
find_library(MULL_DEPENDENCY_LIBZ z HINTS ${MULL_TOOLCHAIN_LIBZ_PATH})
find_library(MULL_DEPENDENCY_NCURSES ncurses)
## Mull's own dependencies:
find_library(MULL_DEPENDENCY_SQLITE sqlite3 HINTS ${MULL_TOOLCHAIN_SQLITE_PATH})
find_library(MULL_DEPENDENCY_LIBCLANG clang HINTS ${LLVM_LIB_PATH})
find_library(MULL_DEPENDENCY_LIBCLANG_TOOLING clangTooling HINTS ${LLVM_LIB_PATH})
find_library(MULL_DEPENDENCY_LIBCLANG_BASIC clangBasic HINTS ${LLVM_LIB_PATH})



include_directories(${MULL_INCLUDE_DIR})

if(MULL_SUPPORT_RUST)
  set(rust_cmake_path "${CMAKE_CURRENT_SOURCE_DIR}/.mull.rust.cmake")
  IF(NOT EXISTS ${rust_cmake_path})
    message(FATAL_ERROR "MULL_SUPPORT_RUST is enabled but .mull.rust.cmake \
      file was not found at path: \
      ${rust_cmake_path}. Please take \
      .mull.rust.cmake.example and modify according to your system")
  endif()

  include(${rust_cmake_path})

  message(STATUS "Mull.Rust> Rust lib path: ${mull_rust_libpath}")
  add_definitions(-DMULL_SUPPORT_RUST=1)
endif()

add_subdirectory(lib)
add_subdirectory(googletest)
add_subdirectory(tools)
# add_subdirectory(test)
add_subdirectory(unittests)
add_subdirectory(projects)
