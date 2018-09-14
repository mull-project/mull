set (SOURCES
  LLVMCompatibility.cpp
  LLVMCompatibility.h
)

add_library(LLVMCompatibility OBJECT ${SOURCES})
set_target_properties(LLVMCompatibility PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
target_include_directories(LLVMCompatibility PUBLIC
  ${LLVM_INCLUDE_DIRS}
)

