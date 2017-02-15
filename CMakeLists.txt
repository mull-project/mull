cmake_minimum_required(VERSION 3.7)

project(Mull)
enable_language(C CXX)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

set(LLVM_DIR "/usr/local/opt/llvm")
set(LLVM_LIB_PATH "${LLVM_DIR}/lib")

set(MULL_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MULL_INCLUDE_DIR  ${MULL_SOURCE_DIR}/include)
set(MULL_TOOLS_DIR    ${LLVM_TOOLS_BINARY_DIR})
set(MULL_BINARY_DIR   ${LLVM_BINARY_DIR})

include_directories("${LLVM_DIR}/include")
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

find_library(MULL_DEPENDENCY_LIBZ z HINTS "/usr/local/opt/zlib/lib")
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
find_library(MULL_DEPENDENCY_SQLITE sqlite3 HINTS "/usr/local/opt/sqlite")

# message(STATUS "MULL_DEPENDENCY_LLVMAsmPrinter - ${MULL_DEPENDENCY_LLVMAsmPrinter}")

include_directories(${MULL_INCLUDE_DIR})

add_subdirectory(tools)
add_subdirectory(lib)
# add_subdirectory(test)
add_subdirectory(unittests)
