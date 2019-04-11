compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/main.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)

