set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/mull-cxx.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions/CLIOptions.cpp
)

add_mull_executable(
  NAME mull-cxx-${LLVM_VERSION_MAJOR}
  SOURCES ${SOURCES}
  LINK_WITH ebc mull json11 LLVM
)
target_include_directories(mull-cxx-${LLVM_VERSION_MAJOR} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions)
