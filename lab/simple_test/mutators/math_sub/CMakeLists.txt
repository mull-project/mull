set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/math_sub.c
)

generate_fixtures(
  TARGET math_sub_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures math_sub_fixtures)


