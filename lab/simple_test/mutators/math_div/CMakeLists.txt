set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/math_div.c
)

generate_fixtures(
  TARGET math_div_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures math_div_fixtures)

