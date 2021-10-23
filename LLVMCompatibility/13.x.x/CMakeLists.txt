set (SOURCES
  LLVMCompatibility.cpp
  LLVMCompatibility.h
)

add_library(LLVMCompatibility OBJECT ${SOURCES})
set_target_properties(LLVMCompatibility PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
target_include_directories(LLVMCompatibility SYSTEM PUBLIC
  ${LLVM_INCLUDE_DIRS}
)
add_dependencies(LLVMCompatibility ${MULL_LLVM_COMPATIBILITY_LIBRARIES})