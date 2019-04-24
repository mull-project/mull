set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/mutator-validator.cpp
)

add_mull_internal_executable(
  SOURCES ${SOURCES}
  NAME mull-mutator-validator
  LINK_WITH mull
)

