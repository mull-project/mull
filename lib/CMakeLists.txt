llvm_add_library(mull
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
  MutationEngine.cpp
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

  ADDITIONAL_HEADER_DIRS
  ${MULL_INCLUDE_DIR}
  ${MULL_INCLUDE_DIR}/MutationOperators
  ${MULL_INCLUDE_DIR}/Context
  ${MULL_INCLUDE_DIR}/GoogleTest
  ${MULL_INCLUDE_DIR}/SimpleTest
  ${MULL_INCLUDE_DIR}/Toolchain

  LINK_LIBS
  LLVMAsmParser
  LLVMAsmPrinter
  LLVMBitReader
  LLVMCodeGen
  LLVMCore
  LLVMExecutionEngine
  LLVMObject
  LLVMObjectYAML
  LLVMOrcJIT
  LLVMRuntimeDyld
  LLVMSupport
  LLVMTarget
  LLVMTransformUtils
  sqlite3

  # FIXME: Should not be arch specific
  LLVMX86AsmParser
  LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
)

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
