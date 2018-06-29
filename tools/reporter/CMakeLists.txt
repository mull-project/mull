set(llvm_components "orcjit")
llvm_get_libs(llvm_libraries ${llvm_components})
string(REPLACE ";" " " llvm_libraries "${llvm_libraries}")
add_executable(mull-reporter
  WeakTestsReporter.h
  WeakTestsReporter.cpp

  reporter.cpp
)
set_target_properties(mull-reporter PROPERTIES
  LINK_FLAGS "${LLVM_LINK_FLAGS} ${llvm_libraries} -lsqlite3"
  COMPILE_FLAGS "-fno-rtti"
)

INSTALL(TARGETS mull-reporter
  RUNTIME DESTINATION bin
)

