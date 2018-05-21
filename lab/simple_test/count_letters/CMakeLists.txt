set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/count_letters.c
  ${CMAKE_CURRENT_LIST_DIR}/test_count_letters.c
)

generate_fixtures(
  TARGET count_letters_fixtures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures count_letters_fixtures)

