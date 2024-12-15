compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/module.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)

