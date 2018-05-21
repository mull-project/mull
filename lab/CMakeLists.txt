include(${CMAKE_CURRENT_LIST_DIR}/lab.cmake)

add_fixtures_target(fixtures)

add_subdirectory(custom_test)
add_subdirectory(simple_test)

set(FIXTURES_FACTORY_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
generate_fixtures_cpp(fixtures)

