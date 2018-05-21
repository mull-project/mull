set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/tester.c
  ${CMAKE_CURRENT_LIST_DIR}/testee.c
)

generate_fixtures(
  TARGET negate_condition_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures negate_condition_fixtures)

