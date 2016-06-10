add_llvm_executable(mutang-driver driver.cpp)

target_link_libraries(mutang-driver
  mutang
  LLVMCore
  LLVMSupport
  LLVMAsmParser
  LLVMBitReader
  )

