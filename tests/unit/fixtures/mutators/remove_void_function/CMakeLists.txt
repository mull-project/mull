compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/junk.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm -I${CMAKE_CURRENT_LIST_DIR}
)
