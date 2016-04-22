add_mutang_unittest(TestFindersTests
  SimpleTestFinderTest.cpp
)

target_link_libraries(TestFindersTests
  mutang
  LLVMCore
  LLVMAsmParser
  LLVMSupport
)

