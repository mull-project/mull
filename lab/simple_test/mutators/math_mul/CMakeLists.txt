set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/math_mul.c
)

generate_fixtures(
  TARGET math_mul_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures math_mul_fixtures)

