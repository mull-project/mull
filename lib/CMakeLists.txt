llvm_add_library(mutang 
  Laboratory.cpp
  TestFinders/SimpleTestFinder.cpp
  MutationOperators/AddMutationOperator.cpp
  MutationEngine.cpp
  Context.cpp
  TestRunners/SimpleTestRunner.cpp

  ADDITIONAL_HEADER_DIRS
  ${MUTANG_MAIN_INCLUDE_DIR}
  ${MUTANG_MAIN_INCLUDE_DIR}/TestFinders
  ${MUTANG_MAIN_INCLUDE_DIR}/TestRunners
  ${MUTANG_MAIN_INCLUDE_DIR}/MutationOperators
  ${MUTANG_MAIN_INCLUDE_DIR}/MutationEngine
  ${MUTANG_MAIN_INCLUDE_DIR}/Context

  LINK_LIBS
  LLVMAsmPrinter
  LLVMCodeGen
  LLVMCore
  LLVMExecutionEngine
  LLVMMCJIT
  LLVMOrcJIT
  LLVMSupport
  LLVMTarget
  # FIXME: Should not be arch specific
  LLVMX86AsmParser
  LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
)

