set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/scalar_value.c
)

generate_fixtures(
  TARGET scalar_value_fxitures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures scalar_value_fxitures)

