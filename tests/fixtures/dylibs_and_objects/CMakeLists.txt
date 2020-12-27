set (main_c ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
set (test_c ${CMAKE_CURRENT_SOURCE_DIR}/test.c)
set (distance_c ${CMAKE_CURRENT_SOURCE_DIR}/distance.c)

compile_fixture(
  INPUT ${test_c}
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm
)

compile_fixture(
  INPUT ${main_c}
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm
)

compile_fixture(
  INPUT ${distance_c}
  OUTPUT_EXTENSION dylib
  FLAGS -shared
)

compile_fixture(
  INPUT ${test_c}
  OUTPUT_EXTENSION o
  FLAGS -c -mcmodel=large
)

