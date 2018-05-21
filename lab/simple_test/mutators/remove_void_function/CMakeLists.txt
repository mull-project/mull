set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/tester.c
  ${CMAKE_CURRENT_LIST_DIR}/testee.c
)

generate_fixtures(
  TARGET remove_void_function_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures remove_void_function_fixtures)

