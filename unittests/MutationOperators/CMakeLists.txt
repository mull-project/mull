add_mutang_unittest(MutationOperatorsTests
  MutationOperatorsTests.cpp
)

target_link_libraries(MutationOperatorsTests
  mutang
  LLVMCore
  LLVMAsmParser
  LLVMSupport
)

