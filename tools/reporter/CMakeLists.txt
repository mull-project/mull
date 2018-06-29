add_executable(mull-reporter
  WeakTestsReporter.h
  WeakTestsReporter.cpp

  reporter.cpp
)
set_target_properties(mull-reporter PROPERTIES
  LINK_FLAGS "${LLVM_LINK_FLAGS} -lLLVMSupport -lsqlite3"
  COMPILE_FLAGS "-fno-rtti"
)

INSTALL(TARGETS mull-reporter
  RUNTIME DESTINATION bin
)

