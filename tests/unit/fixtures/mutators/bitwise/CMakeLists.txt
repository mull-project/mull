compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/shifts.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)
compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/bitops.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)
