compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/main.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm -I ${CMAKE_CURRENT_LIST_DIR}/include
)

set (DIR ${CMAKE_CURRENT_LIST_DIR})

set (INC ${CMAKE_CURRENT_LIST_DIR})
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/absolute.dir/compile_commands.json
)

add_fixture(${CMAKE_CURRENT_BINARY_DIR}/absolute.dir)

set (INC .)
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/relative.dir/compile_commands.json
)
add_fixture(${CMAKE_CURRENT_BINARY_DIR}/relative.dir)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/include/header.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/header.h
  COPYONLY
)

add_fixture(${CMAKE_CURRENT_LIST_DIR}/include)

