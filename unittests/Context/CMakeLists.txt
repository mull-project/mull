add_mutang_unittest(ContextTest
  ContextTest.cpp
)

target_link_libraries(ContextTest
  mutang
  LLVMCore
  LLVMAsmParser
)

