compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/main.c
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)
