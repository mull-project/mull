add_llvm_executable(mutang-driver driver.cpp)

target_link_libraries(mutang-driver
  mutang
  LLVMCore
  LLVMSupport
  LLVMAsmParser
  LLVMBitReader

  # FIXME: Should not be arch specific
  # These are probably should be part of library
  # not of the driver
  LLVMX86AsmParser
  LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
)
