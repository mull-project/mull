set(mull_sources
  ConfigParser.cpp
  Context.cpp
  Driver.cpp
  ForkProcessSandbox.cpp
  Logger.cpp
  ModuleLoader.cpp

  MutationOperators/AddMutationOperator.cpp
  MutationOperators/NegateConditionMutationOperator.cpp
  MutationOperators/RemoveVoidFunctionMutationOperator.cpp

  Toolchain/Compiler.cpp
  Toolchain/ObjectCache.cpp
  Toolchain/Toolchain.cpp

  MullModule.cpp
  MutationPoint.cpp
  Result.cpp
  TestResult.cpp
  TestRunner.cpp
  Testee.cpp

  SimpleTest/SimpleTest_Test.cpp
  SimpleTest/SimpleTestFinder.cpp
  SimpleTest/SimpleTestRunner.cpp

  GoogleTest/GoogleTest_Test.cpp
  GoogleTest/GoogleTestFinder.cpp
  GoogleTest/GoogleTestRunner.cpp

  SQLiteReporter.cpp
)

set(mull_header_dirs
  ${MULL_INCLUDE_DIR}
  ${MULL_INCLUDE_DIR}/MutationOperators
  ${MULL_INCLUDE_DIR}/Context
  ${MULL_INCLUDE_DIR}/GoogleTest
  ${MULL_INCLUDE_DIR}/SimpleTest
  ${MULL_INCLUDE_DIR}/Toolchain
)

set(mull_additional_libraries

  # Architecture-independent:
  ${MULL_DEPENDENCY_LLVMAnalysis}
  ${MULL_DEPENDENCY_LLVMAsmParser}
  ${MULL_DEPENDENCY_LLVMAsmPrinter}
  ${MULL_DEPENDENCY_LLVMBitReader}
  ${MULL_DEPENDENCY_LLVMCodeGen}
  ${MULL_DEPENDENCY_LLVMCore}
  ${MULL_DEPENDENCY_LLVMDebugInfoCodeView}
  ${MULL_DEPENDENCY_LLVMExecutionEngine}
  ${MULL_DEPENDENCY_LLVMMC}
  ${MULL_DEPENDENCY_LLVMMCDisassembler}
  ${MULL_DEPENDENCY_LLVMMCParser}
  ${MULL_DEPENDENCY_LLVMObject}
  ${MULL_DEPENDENCY_LLVMOrcJIT}
  ${MULL_DEPENDENCY_LLVMRuntimeDyld}
  ${MULL_DEPENDENCY_LLVMScalarOpts}
  ${MULL_DEPENDENCY_LLVMSelectionDAG}
  ${MULL_DEPENDENCY_LLVMSupport}
  ${MULL_DEPENDENCY_LLVMTarget}
  ${MULL_DEPENDENCY_LLVMTransformUtils}

  ${MULL_DEPENDENCY_LIBZ}
  ${MULL_DEPENDENCY_NCURSES}

  ${MULL_DEPENDENCY_SQLITE}

  # Architecture-specific:
  ${MULL_DEPENDENCY_LLVMX86AsmParser}
  ${MULL_DEPENDENCY_LLVMX86AsmPrinter}
  ${MULL_DEPENDENCY_LLVMX86CodeGen}
  ${MULL_DEPENDENCY_LLVMX86Desc}
  ${MULL_DEPENDENCY_LLVMX86Info}
  ${MULL_DEPENDENCY_LLVMX86Utils}
)

set(mull_link_flags "") # to be filled later
set(mull_rpath "")      # to be filled later

if(MULL_SUPPORT_RUST)
  message(STATUS "Mull> MULL_SUPPORT_RUST detected.")

  set(mull_sources
    ${mull_sources}
    Rust/RustTest.cpp
    Rust/RustTestFinder.cpp
    Rust/RustTestRunner.cpp)

  set(mull_header_dirs
    ${mull_header_dirs}
    ${MULL_INCLUDE_DIR}/Rust)

  IF(NOT EXISTS ${MULL_RUST_LIB_PATH})
    message(FATAL_ERROR
      "MULL_RUST_LIB_PATH variable should be set to a valid \
      existing path in .mull.rust.cmake")
  endif()

  foreach(library ${MULL_DEPENDENCY_RUST_ADDITIONAL_LIBRARIES})
    IF(NOT EXISTS "${library}")
      message(FATAL_ERROR "Mull.Rust> path declared in \
        MULL_DEPENDENCY_RUST_ADDITIONAL_LIBRARIES does not exist: \
        ${library}")
    endif()
  endforeach()

  set(mull_additional_libraries
    ${mull_additional_libraries}
    ${MULL_DEPENDENCY_RUST_ADDITIONAL_LIBRARIES})

  set(mull_rpath ${mull_rpath} ${MULL_RUST_LIB_PATH})
  set(mull_link_flags ${mull_link_flags} -L${MULL_RUST_LIB_PATH})
endif()

add_library(mull SHARED ${mull_sources})
target_link_libraries(mull ${mull_additional_libraries})

## Once we have mull target added, we can start querying and patching it.
# rpath
get_target_property(default_rpath mull INSTALL_RPATH)
set(mull_rpath ${default_rpath} ${mull_rpath})
set_target_properties(mull PROPERTIES INSTALL_RPATH "${mull_rpath}")

# Link flags
get_target_property(default_link_flags mull LINK_FLAGS)
if(NOT ("default_link_flags-NOTFOUND" STREQUAL ${default_link_flags}))
  set(mull_link_flags ${default_link_flags} ${mull_link_flags})
endif()
set_target_properties(mull PROPERTIES LINK_FLAGS "${mull_link_flags}")

# The following is needed to make CMake to generate a folder structure in Xcode
# that corresponds to a physical folder structure.
get_target_property(HERE_TARGET_SOURCES mull SOURCES)
foreach(source_file ${HERE_TARGET_SOURCES})
  if (source_file MATCHES "(.*)\.h$")
    file(RELATIVE_PATH relative_source_file ${MULL_INCLUDE_DIR} ${source_file})
    get_filename_component(relative_path ${relative_source_file} DIRECTORY)
    source_group("Header Files\\${relative_path}" FILES ${source_file})
  else()
    get_filename_component(relative_path ${source_file} DIRECTORY)
    source_group("Source Files\\${relative_path}" FILES ${source_file})
  endif()
endforeach()
