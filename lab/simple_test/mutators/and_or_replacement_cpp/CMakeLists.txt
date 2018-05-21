set(SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/test_and_or_operators.cpp
)

generate_fixtures(
  TARGET and_or_replacement_cpp_fixtures
  SOURCES ${SOURCES}
  CXXFLAGS -g
)
add_dependencies(fixtures and_or_replacement_cpp_fixtures)

