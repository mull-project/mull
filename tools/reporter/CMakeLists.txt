set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/WeakTestsReporter.h
  ${CMAKE_CURRENT_LIST_DIR}/WeakTestsReporter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/reporter.cpp
)

if ("LLVM" IN_LIST LLVM_AVAILABLE_LIBS)
  set (LINK_WITH ${SQLITE_DEPENDENCY} LLVM)
else()
  set (LINK_WITH ${SQLITE_DEPENDENCY} LLVMSupport)
endif()

add_mull_executable(
  SOURCES ${SOURCES}
  NAME mull-reporter
  LINK_WITH ${LINK_WITH}
)

