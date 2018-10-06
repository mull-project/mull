add_executable(mull-reporter
  WeakTestsReporter.h
  WeakTestsReporter.cpp

  reporter.cpp
)
set_target_properties(mull-reporter PROPERTIES
  LINK_FLAGS ${MULL_LINK_FLAGS}
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)

if ("LLVM" IN_LIST LLVM_AVAILABLE_LIBS)
  set (MULL_LLVM_LIBRARIES
    LLVM
  )
else()
  set (MULL_LLVM_LIBRARIES
    LLVMSupport
  )
endif()

target_link_libraries(mull-reporter
  ${MULL_LLVM_LIBRARIES}
  sqlite3
)
target_include_directories(mull-reporter PUBLIC
  ${MULL_INCLUDE_DIRS}
)

INSTALL(TARGETS mull-reporter
  RUNTIME DESTINATION bin
)

