set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/mull-runner.cpp
  ${CMAKE_CURRENT_LIST_DIR}/MutantExtractor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions/CLIOptions.cpp
)

add_mull_executable(
  NAME mull-runner
  SOURCES ${SOURCES}
  LINK_WITH mull json11
)
target_include_directories(mull-runner PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions)
