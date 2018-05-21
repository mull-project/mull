set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/replace_call.c
)

generate_fixtures(
  TARGET replace_call_fxitures
  SOURCES ${SOURCES}
)
add_dependencies(fixtures replace_call_fxitures)


