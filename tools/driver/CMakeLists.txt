set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/driver.cpp
)

add_mull_executable(
  SOURCES ${SOURCES}
  NAME mull-driver
  LINK_WITH mull
)

