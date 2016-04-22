llvm_add_library(mutang 
  Laboratory.cpp
  TestFinders/SimpleTestFinder.cpp
  Context.cpp

  ADDITIONAL_HEADER_DIRS
  ${MUTANG_MAIN_INCLUDE_DIR}
  ${MUTANG_MAIN_INCLUDE_DIR}/TestFinders

  LINK_LIBS
  LLVMCore
  LLVMSupport
)
