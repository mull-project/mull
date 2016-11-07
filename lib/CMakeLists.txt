llvm_add_library(mutang
  Compiler.cpp
  ConfigParser.cpp
  Context.cpp
  Driver.cpp
  ForkProcessSandbox.cpp
  ModuleLoader.cpp

  MutationOperators/AddMutationOperator.cpp
  MutationOperators/NegateConditionMutationOperator.cpp

  MutangModule.cpp
  MutationEngine.cpp
  MutationPoint.cpp
  TestResult.cpp
  TestRunner.cpp

  SimpleTest/SimpleTest_Test.cpp
  SimpleTest/SimpleTestFinder.cpp
  SimpleTest/SimpleTestRunner.cpp

  GoogleTest/GoogleTest_Test.cpp
  GoogleTest/GoogleTestFinder.cpp
  GoogleTest/GoogleTestRunner.cpp

  SQLiteReporter.cpp

  ADDITIONAL_HEADER_DIRS
  ${MUTANG_INCLUDE_DIR}
  ${MUTANG_INCLUDE_DIR}/MutationOperators
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
  LLVMTransformUtils
  sqlite3

  # FIXME: Should not be arch specific
  LLVMX86AsmParser
  LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
)
