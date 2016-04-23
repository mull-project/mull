add_mutang_unittest(MutationEngineTests
  MutationEngineTests.cpp
)

target_link_libraries(MutationEngineTests
  mutang
  LLVMCore
  LLVMAsmParser
  LLVMSupport
  LLVMTransformUtils
)

