llvm_add_library(mutang 
  Laboratory.cpp
  TestFinders/SimpleTestFinder.cpp
  MutationOperators/AddMutationOperator.cpp
  Context.cpp

  ADDITIONAL_HEADER_DIRS
  ${MUTANG_MAIN_INCLUDE_DIR}
  ${MUTANG_MAIN_INCLUDE_DIR}/TestFinders
  ${MUTANG_MAIN_INCLUDE_DIR}/MutationOperators
  ${MUTANG_MAIN_INCLUDE_DIR}/Context

  LINK_LIBS
  LLVMCore
  LLVMSupport
)
