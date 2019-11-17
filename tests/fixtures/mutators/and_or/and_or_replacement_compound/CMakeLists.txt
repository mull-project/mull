compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/module.c
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)

