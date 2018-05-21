set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/test_and_or_operators.c
)

generate_fixtures(
  TARGET and_or_replacement_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures and_or_replacement_fixtures)

