cmake_minimum_required(VERSION 3.7)

project(Mull)
enable_language(C CXX)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
message(FATAL_ERROR "This CMakeLists.txt file expects cmake to be called with a CMake toolchain file:
cmake ... -DCMAKE_TOOLCHAIN_FILE=path_to_your_toolchain_file.
See example: Mull.toolchain.OSX.cmake")
endif()

if (NOT DEFINED LLVM_ROOT)
  message(FATAL "This CMakeLists.txt file expects cmake to be called with a
    CMake toolchain file which provides LLVM_ROOT:\n \
    -DCMAKE_TOOLCHAIN_FILE=path_to_your_toolchain")
endif()

set(LLVM_LIB_PATH "${LLVM_ROOT}/lib")

set(MULL_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MULL_INCLUDE_DIR  ${MULL_SOURCE_DIR}/include)
set(MULL_CC  "${MULL_SOURCE_DIR}/mull-clang")
set(MULL_CXX "${MULL_SOURCE_DIR}/mull-clang++")
set(MULL_TMP "${MULL_SOURCE_DIR}/tmp")

include_directories("${LLVM_ROOT}/include")
include_directories("${MULL_SOURCE_DIR}/googletest")

## LLVM dependencies:
# Architecture-independent files.
find_library(MULL_DEPENDENCY_LLVMAnalysis LLVMAnalysis HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMAsmParser LLVMAsmParser HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMAsmPrinter LLVMAsmPrinter HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMBitReader LLVMBitReader HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMCodeGen LLVMCodeGen HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMCore LLVMCore HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMDebugInfoCodeView LLVMDebugInfoCodeView HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMExecutionEngine LLVMExecutionEngine HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMMC LLVMMC HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMMCDisassembler LLVMMCDisassembler HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMMCParser LLVMMCParser HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMObject LLVMObject HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMOrcJIT LLVMOrcJIT HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMRuntimeDyld LLVMRuntimeDyld HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMScalarOpts LLVMScalarOpts HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMSelectionDAG LLVMSelectionDAG HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMSupport LLVMSupport HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMTarget LLVMTarget HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMTransformUtils LLVMTransformUtils HINTS "${LLVM_LIB_PATH}")

if (NOT DEFINED MULL_TOOLCHAIN_ZLIB_PATH)
message(SEND_ERROR "MULL_TOOLCHAIN_ZLIB_PATH variable must be provided in a toolchain file.
Example: set(MULL_TOOLCHAIN_ZLIB_PATH \"/usr/local/opt/zlib/lib\")")
endif()
find_library(MULL_DEPENDENCY_LIBZ z HINTS ${MULL_TOOLCHAIN_LIBZ_PATH})
find_library(MULL_DEPENDENCY_NCURSES ncurses)

find_library(MULL_DEPENDENCY_SQLITE sqlite3 HINTS "/usr/local/opt/sqlite")

find_library(MULL_DEPENDENCY_GOOGLETEST gtest HINTS "${MULL_SOURCE_DIR}/googletest")
find_library(MULL_DEPENDENCY_GOOGLETESTMAIN gtest_main HINTS "${MULL_SOURCE_DIR}/googletest")

# Architecture-specific files.
find_library(MULL_DEPENDENCY_LLVMX86AsmParser LLVMX86AsmParser HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMX86AsmPrinter LLVMX86AsmPrinter HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMX86CodeGen LLVMX86CodeGen HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMX86Desc LLVMX86Desc HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMX86Info LLVMX86Info HINTS "${LLVM_LIB_PATH}")
find_library(MULL_DEPENDENCY_LLVMX86Utils LLVMX86Utils HINTS "${LLVM_LIB_PATH}")

## Mull's own dependencies:
if (NOT DEFINED MULL_TOOLCHAIN_SQLITE_PATH)
message(SEND_ERROR "MULL_TOOLCHAIN_SQLITE_PATH variable must be provided in a toolchain file.
Example: set(MULL_TOOLCHAIN_SQLITE_PATH \"/usr/local/opt/sqlite\"")
endif()
find_library(MULL_DEPENDENCY_SQLITE sqlite3 HINTS ${MULL_TOOLCHAIN_SQLITE_PATH})

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
add_subdirectory(tools)
# add_subdirectory(test)
add_subdirectory(unittests)

include("Mull.extensions.cmake")

mull_clone_target(mull mull-bitcode)
set_target_properties(mull-bitcode PROPERTIES XCODE_ATTRIBUTE_CC "${MULL_CC}")
set_target_properties(mull-bitcode PROPERTIES XCODE_ATTRIBUTE_CXX "${MULL_CXX}")

mull_clone_target(MullUnitTests mull-unittests-bitcode)
set_target_properties(mull-unittests-bitcode PROPERTIES XCODE_ATTRIBUTE_CC "${MULL_CC}")
set_target_properties(mull-unittests-bitcode PROPERTIES XCODE_ATTRIBUTE_CXX "${MULL_CXX}")

# set(mull_library "$<TARGET_FILE:mull>")
# set(mull_driver_executable "$<TARGET_FILE:mull-driver>")
# set(mull_config_yml "${MULL_TMP}/config.yml")
mull_clone_executable_target(mull-driver mull-autotest-driver)

add_dependencies(mull-autotest-driver
  mull-bitcode
  mull-unittests-bitcode
)
