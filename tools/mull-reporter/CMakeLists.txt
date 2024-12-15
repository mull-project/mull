set(SOURCES ${CMAKE_CURRENT_LIST_DIR}/mull-reporter.cpp
            ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions/CLIOptions.cpp)

add_mull_executable(
  NAME
  mull-reporter-${LLVM_VERSION_MAJOR}
  SOURCES
  ${SOURCES}
  LINK_WITH
  mull
  json11
  LLVM)
target_include_directories(mull-reporter-${LLVM_VERSION_MAJOR}
                           PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions)
