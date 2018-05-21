set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/replace_assignment.c
)

generate_fixtures(
  TARGET replace_assignment_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures replace_assignment_fixtures)


