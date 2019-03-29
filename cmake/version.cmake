execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file (
  ${CMAKE_SOURCE_DIR}/lib/Version.cpp
  ${CMAKE_BINARY_DIR}/lib/Version.cpp
  @ONLY
)

add_library(MullVersion ${CMAKE_BINARY_DIR}/lib/Version.cpp)
target_include_directories(MullVersion PRIVATE
  ${MULL_INCLUDE_DIRS}
)
set_target_properties(MullVersion PROPERTIES
  LINK_FLAGS ${MULL_LINK_FLAGS}
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
