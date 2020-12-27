compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/module.c
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)
compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/main.c
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)

compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/junk.cpp
  OUTPUT_EXTENSION bc
  FLAGS -g -c -emit-llvm
)

compile_fixture(
  INPUT ${CMAKE_CURRENT_LIST_DIR}/junk.cpp
  OUTPUT_EXTENSION ll
  FLAGS -g -c -S -emit-llvm
)
