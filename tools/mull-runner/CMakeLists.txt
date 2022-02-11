set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/mull-runner.cpp
  ${CMAKE_CURRENT_LIST_DIR}/MutantExtractor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/DynamicLibraries.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions/CLIOptions.cpp
  ${CMAKE_CURRENT_LIST_DIR}/MergeInstProfile.cpp

)

add_mull_executable(
  NAME mull-runner-${LLVM_VERSION_MAJOR}
  SOURCES ${SOURCES}
  LINK_WITH mull json11 LLVM
)
target_include_directories(mull-runner-${LLVM_VERSION_MAJOR} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions)
