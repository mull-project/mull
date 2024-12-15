compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/main.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm -I ${CMAKE_CURRENT_LIST_DIR}/include
)

set (DIR ${CMAKE_CURRENT_LIST_DIR})

set (INC ${CMAKE_CURRENT_LIST_DIR})
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/absolute_compile_commands.json
)

add_fixture(${CMAKE_CURRENT_BINARY_DIR}/absolute_compile_commands.json)

set (INC .)
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/relative_compile_commands.json
)
add_fixture(${CMAKE_CURRENT_BINARY_DIR}/relative_compile_commands.json)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/include/header.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/header.h
  COPYONLY
)

add_fixture(${CMAKE_CURRENT_LIST_DIR}/include)

set (databases
  ${CMAKE_CURRENT_LIST_DIR}/db_with_arguments.json
  ${CMAKE_CURRENT_LIST_DIR}/db_with_commands.json
  ${CMAKE_CURRENT_LIST_DIR}/db_with_output.json
  ${CMAKE_CURRENT_LIST_DIR}/db_with_fullpath_compiler.json
  ${CMAKE_CURRENT_LIST_DIR}/db_with_escaped_quotes.json
  ${CMAKE_CURRENT_LIST_DIR}/db_produced_from_clang_MJ_valid_sequence.json
)

foreach(database ${databases})
  add_fixture(${database})
endforeach()

