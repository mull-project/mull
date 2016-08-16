llvm_add_library(mutang
  Compiler.cpp
  ConfigParser.cpp
  Context.cpp
  Driver.cpp
  ModuleLoader.cpp
  MutationOperators/AddMutationOperator.cpp
  MutationEngine.cpp
  MutationPoint.cpp
  TestResult.cpp
  TestRunners/SimpleTestRunner.cpp
  TestRunners/GoogleTestRunner.cpp

  SimpleTest/SimpleTest_Test.cpp
  SimpleTest/SimpleTestFinder.cpp

  GoogleTest/GoogleTestFinder.cpp

  ADDITIONAL_HEADER_DIRS
  ${MUTANG_INCLUDE_DIR}
  ${MUTANG_INCLUDE_DIR}/TestRunners
  ${MUTANG_INCLUDE_DIR}/MutationOperators
  ${MUTANG_INCLUDE_DIR}/MutationEngine
  ${MUTANG_INCLUDE_DIR}/Context

  LINK_LIBS
  LLVMAsmPrinter
  LLVMAsmParser
  LLVMBitReader
  LLVMCodeGen
  LLVMCore
  LLVMExecutionEngine
  LLVMMCJIT
  LLVMObject
  LLVMOrcJIT
  LLVMRuntimeDyld
  LLVMSupport
  LLVMTarget
  # FIXME: Should not be arch specific
  LLVMX86AsmParser
  LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
)

