compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/bitwise_not.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)
