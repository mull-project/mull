add_mutang_unittest(TestRunnersTests
  TestRunnersTests.cpp
)

target_link_libraries(TestRunnersTests
  mutang
  LLVMCore
  LLVMAsmParser
  LLVMSupport
)

