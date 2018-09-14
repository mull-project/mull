add_executable(mull-reporter
  WeakTestsReporter.h
  WeakTestsReporter.cpp

  reporter.cpp
)
set_target_properties(mull-reporter PROPERTIES
  LINK_FLAGS ${MULL_LINK_FLAGS}
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
target_link_libraries(mull-reporter
  LLVMSupport
  sqlite3
)
target_include_directories(mull-reporter PUBLIC
  ${MULL_INCLUDE_DIRS}
)

INSTALL(TARGETS mull-reporter
  RUNTIME DESTINATION bin
)

