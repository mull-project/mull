set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/distance.c
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/test.c
)

generate_fixtures(
  TARGET custom_test_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures custom_test_fixtures)

