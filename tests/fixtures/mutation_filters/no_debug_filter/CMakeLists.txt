compile_fixture(
  INPUT ${CMAKE_CURRENT_SOURCE_DIR}/with_debug.c
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm -g
)

compile_fixture(
  INPUT ${CMAKE_CURRENT_SOURCE_DIR}/without_debug.c
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm
)
