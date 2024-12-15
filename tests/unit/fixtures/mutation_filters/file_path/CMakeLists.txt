compile_fixture(
  INPUT ${CMAKE_CURRENT_SOURCE_DIR}/some_test_file_name.c
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm -g
)
